/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2012, Joyent, Inc.  All rights reserved.
 */

/*
 * The ipmi driver is an openipmi compatible IPMI driver based on the FreeBSD
 * driver.
 *
 * The current implementation has several limitations:
 * 1) It only does discovery through the SMBIOS.  The FreeBSD driver has
 *    several additional ways to discover the IPMI device (acpi, bus checking,
 *    etc.).  This support could be ported if necessary.
 * 2) The driver currently only supports the IPMI KCS_MODE mode (reported
 *    through the SMBIOS as SMBIOS SMB_IPMI_T_KCS). Support for the other modes
 *    (BT_MODE, SMIC_MODE, SSIF_MODE) could be ported if necessary.
 * 3) The driver does not currently set up an IPMI watchdog.  This also could
 *    be ported if necessary.
 */

#include <sys/devops.h>
#include <sys/conf.h>
#include <sys/modctl.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/open.h>
#include <sys/cred.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/cmn_err.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/smbios.h>
#include <sys/smbios_impl.h>
#include <sys/policy.h>
#include <sys/ipmi.h>
#include "ipmivars.h"

static dev_info_t		*ipmi_dip;
static boolean_t		ipmi_attached = B_FALSE;
static boolean_t		ipmi_found = B_FALSE;
static struct ipmi_softc	softc;
static struct ipmi_softc	*sc = &softc;
static list_t			dev_list;
static id_space_t		*minor_ids;

#define	PTRIN(p)	((void *)(uintptr_t)(p))
#define	PTROUT(p)	((uintptr_t)(p))

/*
 * Use the SMBIOS info to determine if the system has an IPMI.
 */
static void
get_smbios_ipmi_info()
{
	smbios_ipmi_t ipmi;

	if (ksmbios == NULL || smbios_info_ipmi(ksmbios, &ipmi) == SMB_ERR)
		return;

	cmn_err(CE_CONT, "!SMBIOS type 0x%x, addr 0x%llx", ipmi.smbip_type,
	    (long long unsigned int)(ipmi.smbip_addr));

	sc->ipmi_io_type = ipmi.smbip_type;
	switch (ipmi.smbip_type) {
	case SMB_IPMI_T_KCS:
	case SMB_IPMI_T_SMIC:
		sc->ipmi_io_address = ipmi.smbip_addr;
		sc->ipmi_io_mode = (ipmi.smbip_flags & SMB_IPMI_F_IOADDR) ?
		    1 : 0;
		sc->ipmi_io_spacing = ipmi.smbip_regspacing;
		break;
	case SMB_IPMI_T_SSIF:
		if ((ipmi.smbip_addr & 0xffffffffffffff00) != 0) {
			cmn_err(CE_WARN, "!SMBIOS: Invalid SSIF SMBus address, "
			    "using BMC I2C slave address instead");
			sc->ipmi_io_address = ipmi.smbip_i2c;
		} else {
			sc->ipmi_io_address = ipmi.smbip_addr;
		}
		break;
	default:
		return;
	}

	if (ipmi.smbip_intr > 15)
		cmn_err(CE_WARN, "!SMBIOS: Non-ISA IRQ %d for IPMI",
		    ipmi.smbip_intr);
	else
		sc->ipmi_io_irq = ipmi.smbip_intr;
}

static ipmi_device_t *
lookup_ipmidev_by_dev(dev_t dev)
{
	ipmi_device_t	*p;

	for (p = list_head(&dev_list); p; p = list_next(&dev_list, p)) {
		if (dev == p->ipmi_dev)
			return (p);
	}
	return (NULL);
}

/*
 * Each open returns a new pseudo device.
 */
/*ARGSUSED*/
static int
ipmi_open(dev_t *devp, int flag, int otyp, cred_t *cred)
{
	minor_t minor;
	ipmi_device_t *dev;

	if (ipmi_attached == B_FALSE)
		return (ENXIO);

	if (ipmi_found == B_FALSE)
		return (ENODEV);

	/* exclusive opens are not supported */
	if (flag & FEXCL)
		return (ENOTSUP);

	if ((minor = (minor_t)id_alloc_nosleep(minor_ids)) == 0)
		return (ENODEV);

	/* Initialize the per file descriptor data. */
	dev = kmem_zalloc(sizeof (ipmi_device_t), KM_SLEEP);

	dev->ipmi_pollhead = kmem_zalloc(sizeof (pollhead_t), KM_SLEEP);

	TAILQ_INIT(&dev->ipmi_completed_requests);
	dev->ipmi_address = IPMI_BMC_SLAVE_ADDR;
	dev->ipmi_lun = IPMI_BMC_SMS_LUN;
	*devp = makedevice(getmajor(*devp), minor);
	dev->ipmi_dev = *devp;

	list_insert_head(&dev_list, dev);

	return (0);
}

/*ARGSUSED*/
static int
ipmi_close(dev_t dev, int flag, int otyp, cred_t *cred)
{
	ipmi_device_t *dp;
	struct ipmi_request *req;

	if ((dp = lookup_ipmidev_by_dev(dev)) == NULL)
		return (ENODEV);

	IPMI_LOCK(sc);
	/* remove any pending requests */
restart:
	for (req = TAILQ_FIRST(&sc->ipmi_pending_requests); req != NULL;
	    req = TAILQ_NEXT(req, ir_link)) {
		if (req->ir_owner == dp) {
			TAILQ_REMOVE(&sc->ipmi_pending_requests, req, ir_link);
			ipmi_free_request(req);
			goto restart;
		}
	}
	IPMI_UNLOCK(sc);

	/* remove any requests in queue of stuff completed */
	while ((req = TAILQ_FIRST(&dp->ipmi_completed_requests)) != NULL) {
		TAILQ_REMOVE(&dp->ipmi_completed_requests, req, ir_link);
		ipmi_free_request(req);
	}

	list_remove(&dev_list, dp);
	id_free(minor_ids, getminor(dev));
	kmem_free(dp->ipmi_pollhead, sizeof (pollhead_t));
	kmem_free(dp, sizeof (ipmi_device_t));

	return (0);
}

/*ARGSUSED*/
static int
ipmi_ioctl(dev_t dv, int cmd, intptr_t data, int flags, cred_t *cr, int *rvalp)
{
	struct ipmi_device *dev;
	struct ipmi_request *kreq;
	struct ipmi_req req;
	struct ipmi_recv recv;
	struct ipmi_recv32 recv32;
	struct ipmi_addr addr;
	int error, len;
	model_t model;
	int orig_cmd = 0;
	uchar_t	t_lun;

	if (secpolicy_sys_config(cr, B_FALSE) != 0)
		return (EPERM);

	if ((dev = lookup_ipmidev_by_dev(dv)) == NULL)
		return (ENODEV);

	model = get_udatamodel();
	if (model == DATAMODEL_NATIVE) {
		switch (cmd) {
		case IPMICTL_SEND_COMMAND:
			if (copyin((void *)data, &req, sizeof (req)))
				return (EFAULT);
			break;
		case IPMICTL_RECEIVE_MSG_TRUNC:
		case IPMICTL_RECEIVE_MSG:
			if (copyin((void *)data, &recv, sizeof (recv)))
				return (EFAULT);
			break;
		}
	} else {
		/* Convert 32-bit structures to native. */
		struct ipmi_req32 req32;

		switch (cmd) {
		case IPMICTL_SEND_COMMAND_32:
			if (copyin((void *)data, &req32, sizeof (req32)))
				return (EFAULT);

			req.addr = PTRIN(req32.addr);
			req.addr_len = req32.addr_len;
			req.msgid = req32.msgid;
			req.msg.netfn = req32.msg.netfn;
			req.msg.cmd = req32.msg.cmd;
			req.msg.data_len = req32.msg.data_len;
			req.msg.data = PTRIN(req32.msg.data);

			cmd = IPMICTL_SEND_COMMAND;
			break;

		case IPMICTL_RECEIVE_MSG_TRUNC_32:
		case IPMICTL_RECEIVE_MSG_32:
			if (copyin((void *)data, &recv32, sizeof (recv32)))
				return (EFAULT);

			recv.addr = PTRIN(recv32.addr);
			recv.addr_len = recv32.addr_len;
			recv.msg.data_len = recv32.msg.data_len;
			recv.msg.data = PTRIN(recv32.msg.data);

			orig_cmd = cmd;
			cmd = (cmd == IPMICTL_RECEIVE_MSG_TRUNC_32) ?
			    IPMICTL_RECEIVE_MSG_TRUNC : IPMICTL_RECEIVE_MSG;
			break;
		}
	}

	switch (cmd) {
	case IPMICTL_SEND_COMMAND:
		IPMI_LOCK(sc);
		/* clear out old stuff in queue of stuff done */
		while ((kreq = TAILQ_FIRST(&dev->ipmi_completed_requests))
		    != NULL) {
			TAILQ_REMOVE(&dev->ipmi_completed_requests, kreq,
			    ir_link);
			dev->ipmi_requests--;
			ipmi_free_request(kreq);
		}
		IPMI_UNLOCK(sc);

		/* Check that we didn't get a ridiculous length */
		if (req.msg.data_len > IPMI_MAX_RX)
			return (EINVAL);

		kreq = ipmi_alloc_request(dev, req.msgid,
		    IPMI_ADDR(req.msg.netfn, 0), req.msg.cmd,
		    req.msg.data_len, IPMI_MAX_RX);
		/* This struct is the same for 32/64 */
		if (copyin(req.msg.data, kreq->ir_request, req.msg.data_len)) {
			ipmi_free_request(kreq);
			return (EFAULT);
		}
		IPMI_LOCK(sc);
		dev->ipmi_requests++;
		error = sc->ipmi_enqueue_request(sc, kreq);
		IPMI_UNLOCK(sc);
		if (error)
			return (error);
		break;

	case IPMICTL_RECEIVE_MSG_TRUNC:
	case IPMICTL_RECEIVE_MSG:
		/* This struct is the same for 32/64 */
		if (copyin(recv.addr, &addr, sizeof (addr)))
			return (EFAULT);

		IPMI_LOCK(sc);
		kreq = TAILQ_FIRST(&dev->ipmi_completed_requests);
		if (kreq == NULL) {
			IPMI_UNLOCK(sc);
			return (EAGAIN);
		}
		addr.channel = IPMI_BMC_CHANNEL;
		recv.recv_type = IPMI_RESPONSE_RECV_TYPE;
		recv.msgid = kreq->ir_msgid;
		recv.msg.netfn = IPMI_REPLY_ADDR(kreq->ir_addr) >> 2;
		recv.msg.cmd = kreq->ir_command;
		error = kreq->ir_error;
		if (error) {
			TAILQ_REMOVE(&dev->ipmi_completed_requests, kreq,
			    ir_link);
			dev->ipmi_requests--;
			IPMI_UNLOCK(sc);
			ipmi_free_request(kreq);
			return (error);
		}
		len = kreq->ir_replylen + 1;
		if (recv.msg.data_len < len && cmd == IPMICTL_RECEIVE_MSG) {
			IPMI_UNLOCK(sc);
			ipmi_free_request(kreq);
			return (EMSGSIZE);
		}
		TAILQ_REMOVE(&dev->ipmi_completed_requests, kreq, ir_link);
		dev->ipmi_requests--;
		IPMI_UNLOCK(sc);
		len = min(recv.msg.data_len, len);
		recv.msg.data_len = (unsigned short)len;

		if (orig_cmd == IPMICTL_RECEIVE_MSG_TRUNC_32 ||
		    orig_cmd == IPMICTL_RECEIVE_MSG_32) {
			/* Update changed fields in 32-bit structure. */
			recv32.recv_type = recv.recv_type;
			recv32.msgid = (int32_t)recv.msgid;
			recv32.msg.netfn = recv.msg.netfn;
			recv32.msg.cmd = recv.msg.cmd;
			recv32.msg.data_len = recv.msg.data_len;

			error = copyout(&recv32, (void *)data, sizeof (recv32));
		} else {
			error = copyout(&recv, (void *)data, sizeof (recv));
		}

		/* This struct is the same for 32/64 */
		if (error == 0)
			error = copyout(&addr, recv.addr, sizeof (addr));
		if (error == 0)
			error = copyout(&kreq->ir_compcode, recv.msg.data, 1);
		if (error == 0)
			error = copyout(kreq->ir_reply, recv.msg.data + 1,
			    len - 1);
		ipmi_free_request(kreq);

		if (error)
			return (EFAULT);

		break;

	case IPMICTL_SET_MY_ADDRESS_CMD:
		IPMI_LOCK(sc);
		if (copyin((void *)data, &dev->ipmi_address,
		    sizeof (dev->ipmi_address))) {
			IPMI_UNLOCK(sc);
			return (EFAULT);
		}
		IPMI_UNLOCK(sc);
		break;

	case IPMICTL_GET_MY_ADDRESS_CMD:
		IPMI_LOCK(sc);
		if (copyout(&dev->ipmi_address, (void *)data,
		    sizeof (dev->ipmi_address))) {
			IPMI_UNLOCK(sc);
			return (EFAULT);
		}
		IPMI_UNLOCK(sc);
		break;

	case IPMICTL_SET_MY_LUN_CMD:
		IPMI_LOCK(sc);
		if (copyin((void *)data, &t_lun, sizeof (t_lun))) {
			IPMI_UNLOCK(sc);
			return (EFAULT);
		}
		dev->ipmi_lun = t_lun & 0x3;
		IPMI_UNLOCK(sc);
		break;

	case IPMICTL_GET_MY_LUN_CMD:
		IPMI_LOCK(sc);
		if (copyout(&dev->ipmi_lun, (void *)data,
		    sizeof (dev->ipmi_lun))) {
			IPMI_UNLOCK(sc);
			return (EFAULT);
		}
		IPMI_UNLOCK(sc);
		break;

	case IPMICTL_SET_GETS_EVENTS_CMD:
		break;

	case IPMICTL_REGISTER_FOR_CMD:
	case IPMICTL_UNREGISTER_FOR_CMD:
		return (EINVAL);

	default:
		return (EINVAL);
	}

	return (0);
}

static int
ipmi_poll(dev_t dv, short events, int anyyet, short *reventsp,
    pollhead_t **phpp)
{
	struct ipmi_device *dev;
	short revent = 0;

	if ((dev = lookup_ipmidev_by_dev(dv)) == NULL)
		return (ENODEV);

	if (events & (POLLIN | POLLRDNORM)) {
		if (!TAILQ_EMPTY(&dev->ipmi_completed_requests))
			revent |= events & (POLLIN | POLLRDNORM);
		if (dev->ipmi_requests == 0)
			revent |= POLLERR;
	}

	if (revent == 0) {
		/* nothing has occurred */
		if (!anyyet)
			*phpp = dev->ipmi_pollhead;
	}

	*reventsp = revent;
	return (0);
}

/*ARGSUSED*/
static int
ipmi_info(dev_info_t *dip, ddi_info_cmd_t cmd, void *arg, void **resultp)
{
	switch (cmd) {
	case DDI_INFO_DEVT2DEVINFO:
		*resultp = ipmi_dip;
		return (DDI_SUCCESS);
	case DDI_INFO_DEVT2INSTANCE:
		*resultp = NULL;
		return (DDI_SUCCESS);
	}
	return (DDI_FAILURE);
}

static int
ipmi_attach(dev_info_t *dip, ddi_attach_cmd_t cmd)
{
	if (cmd != DDI_ATTACH)
		return (DDI_FAILURE);

	get_smbios_ipmi_info();

	/*
	 * Support for the other types (SMIC, SSIF) should be added here.
	 */
	switch (sc->ipmi_io_type) {
	case SMB_IPMI_T_KCS:
		if (ipmi_kcs_attach(sc) != 0)
			return (DDI_FAILURE);
		break;
	default:
		return (DDI_FAILURE);
	}
	ipmi_found = B_TRUE;

	if (ddi_create_minor_node(dip, "ipmi", S_IFCHR, 0, DDI_PSEUDO,
	    0) == DDI_FAILURE) {
		cmn_err(CE_WARN, "!attach could not create minor node");
		ddi_remove_minor_node(dip, NULL);
		return (DDI_FAILURE);
	}

	ipmi_dip = dip;

	list_create(&dev_list, sizeof (ipmi_device_t),
	    offsetof(ipmi_device_t, ipmi_node));

	/* Create ID space for open devs.  ID 0 is reserved. */
	minor_ids = id_space_create("ipmi_id_space", 1, 128);

	ipmi_startup(sc);
	ipmi_attached = B_TRUE;

	return (DDI_SUCCESS);
}

static int
ipmi_detach(dev_info_t *dip, ddi_detach_cmd_t cmd)
{
	if (cmd != DDI_DETACH)
		return (DDI_FAILURE);

	if (ipmi_found == B_FALSE)
		return (DDI_SUCCESS);

	if (!list_is_empty(&dev_list))
		return (DDI_FAILURE);

	/* poke the taskq so that it can terminate */
	sc->ipmi_detaching = 1;
	cv_signal(&sc->ipmi_request_added);

	ddi_remove_minor_node(dip, NULL);
	ipmi_dip = NULL;

	taskq_destroy(sc->ipmi_kthread);
	list_destroy(&dev_list);
	id_space_destroy(minor_ids);

	ipmi_attached = B_FALSE;
	return (DDI_SUCCESS);
}

static struct cb_ops ipmi_cb_ops = {
	ipmi_open,
	ipmi_close,
	nodev,			/* strategy */
	nodev,			/* print */
	nodev,			/* dump */
	nodev,			/* read */
	nodev,			/* write */
	ipmi_ioctl,
	nodev,			/* devmap */
	nodev,			/* mmap */
	nodev,			/* segmap */
	ipmi_poll,
	ddi_prop_op,
	NULL,			/* streamtab */
	D_NEW | D_MP		/* flags */
};

static struct dev_ops ipmi_ops = {
	DEVO_REV,
	0,			/* reference count */
	ipmi_info,
	nulldev,		/* identify */
	nulldev,		/* probe */
	ipmi_attach,
	ipmi_detach,
	nodev,			/* reset */
	&ipmi_cb_ops,
	NULL,			/* bus ops */
	NULL,			/* power */
	ddi_quiesce_not_needed,
};

static struct modldrv md = {
	&mod_driverops, "ipmi driver", &ipmi_ops
};

static struct modlinkage ml = {
	MODREV_1, &md, NULL
};

int
_init(void)
{
	return (mod_install(&ml));
}

int
_fini(void)
{
	return (mod_remove(&ml));
}

int
_info(struct modinfo *mip)
{
	return (mod_info(&ml, mip));
}
