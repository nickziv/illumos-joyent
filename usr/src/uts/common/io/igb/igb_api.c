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
 * Copyright (c) 2007-2012 Intel Corporation. All rights reserved.
 */

/*
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 */

/* IntelVersion: 1.129.2.1 v3_3_14_3_BHSW1 */

#include "igb_api.h"

/*
 * e1000_init_mac_params - Initialize MAC function pointers
 * @hw: pointer to the HW structure
 *
 * This function initializes the function pointers for the MAC
 * set of functions.  Called by drivers or by e1000_setup_init_funcs.
 */
s32
e1000_init_mac_params(struct e1000_hw *hw)
{
	s32 ret_val = E1000_SUCCESS;

	if (hw->mac.ops.init_params) {
		ret_val = hw->mac.ops.init_params(hw);
		if (ret_val) {
			DEBUGOUT("MAC Initialization Error\n");
			goto out;
		}
	} else {
		DEBUGOUT("mac.init_mac_params was NULL\n");
		ret_val = -E1000_ERR_CONFIG;
	}

out:
	return (ret_val);
}

/*
 * e1000_init_nvm_params - Initialize NVM function pointers
 * @hw: pointer to the HW structure
 *
 * This function initializes the function pointers for the NVM
 * set of functions.  Called by drivers or by e1000_setup_init_funcs.
 */
s32
e1000_init_nvm_params(struct e1000_hw *hw)
{
	s32 ret_val = E1000_SUCCESS;

	if (hw->nvm.ops.init_params) {
		ret_val = hw->nvm.ops.init_params(hw);
		if (ret_val) {
			DEBUGOUT("NVM Initialization Error\n");
			goto out;
		}
	} else {
		DEBUGOUT("nvm.init_nvm_params was NULL\n");
		ret_val = -E1000_ERR_CONFIG;
	}

out:
	return (ret_val);
}

/*
 * e1000_init_phy_params - Initialize PHY function pointers
 * @hw: pointer to the HW structure
 *
 * This function initializes the function pointers for the PHY
 * set of functions.  Called by drivers or by e1000_setup_init_funcs.
 */
s32
e1000_init_phy_params(struct e1000_hw *hw)
{
	s32 ret_val = E1000_SUCCESS;

	if (hw->phy.ops.init_params) {
		ret_val = hw->phy.ops.init_params(hw);
		if (ret_val) {
			DEBUGOUT("PHY Initialization Error\n");
			goto out;
		}
	} else {
		DEBUGOUT("phy.init_phy_params was NULL\n");
		ret_val =  -E1000_ERR_CONFIG;
	}

out:
	return (ret_val);
}

/*
 * e1000_set_mac_type - Sets MAC type
 * @hw: pointer to the HW structure
 *
 * This function sets the mac type of the adapter based on the
 * device ID stored in the hw structure.
 * MUST BE FIRST FUNCTION CALLED (explicitly or through
 * e1000_setup_init_funcs()).
 */
s32
e1000_set_mac_type(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val = E1000_SUCCESS;

	DEBUGFUNC("e1000_set_mac_type");

	switch (hw->device_id) {
	case E1000_DEV_ID_82575EB_COPPER:
	case E1000_DEV_ID_82575EB_FIBER_SERDES:
	case E1000_DEV_ID_82575GB_QUAD_COPPER:
		mac->type = e1000_82575;
		break;
	case E1000_DEV_ID_82576:
	case E1000_DEV_ID_82576_FIBER:
	case E1000_DEV_ID_82576_SERDES:
	case E1000_DEV_ID_82576_QUAD_COPPER:
	case E1000_DEV_ID_82576_QUAD_COPPER_ET2:
	case E1000_DEV_ID_82576_NS:
	case E1000_DEV_ID_82576_NS_SERDES:
	case E1000_DEV_ID_82576_SERDES_QUAD:
		mac->type = e1000_82576;
		break;
	case E1000_DEV_ID_82580_COPPER:
	case E1000_DEV_ID_82580_FIBER:
	case E1000_DEV_ID_82580_SERDES:
	case E1000_DEV_ID_82580_SGMII:
	case E1000_DEV_ID_82580_COPPER_DUAL:
		mac->type = e1000_82580;
		break;
	case E1000_DEV_ID_I350_COPPER:
		mac->type = e1000_i350;
		break;
	default:
		/* Should never have loaded on this device */
		ret_val = -E1000_ERR_MAC_INIT;
		break;
	}

	return (ret_val);
}

/*
 * e1000_setup_init_funcs - Initializes function pointers
 * @hw: pointer to the HW structure
 * @init_device: true will initialize the rest of the function pointers
 *		getting the device ready for use.  false will only set
 *		MAC type and the function pointers for the other init
 *		functions.  Passing false will not generate any hardware
 *		reads or writes.
 *
 * This function must be called by a driver in order to use the rest
 * of the 'shared' code files. Called by drivers only.
 */
s32
e1000_setup_init_funcs(struct e1000_hw *hw, bool init_device)
{
	s32 ret_val;

	/* Can't do much good without knowing the MAC type. */
	ret_val = e1000_set_mac_type(hw);
	if (ret_val) {
		DEBUGOUT("ERROR: MAC type could not be set properly.\n");
		goto out;
	}

	if (!hw->hw_addr) {
		DEBUGOUT("ERROR: Registers not mapped\n");
		ret_val = -E1000_ERR_CONFIG;
		goto out;
	}

	/*
	 * Init function pointers to generic implementations. We do this first
	 * allowing a driver module to override it afterward.
	 */
	e1000_init_mac_ops_generic(hw);
	e1000_init_phy_ops_generic(hw);
	e1000_init_nvm_ops_generic(hw);

	/*
	 * Set up the init function pointers. These are functions within the
	 * adapter family file that sets up function pointers for the rest of
	 * the functions in that family.
	 */
	switch (hw->mac.type) {
	case e1000_82575:
	case e1000_82576:
	case e1000_82580:
	case e1000_i350:
		e1000_init_function_pointers_82575(hw);
		break;
	default:
		DEBUGOUT("Hardware not supported\n");
		ret_val = -E1000_ERR_CONFIG;
		break;
	}

	/*
	 * Initialize the rest of the function pointers. These require some
	 * register reads/writes in some cases.
	 */
	if (!(ret_val) && init_device) {
		ret_val = e1000_init_mac_params(hw);
		if (ret_val)
			goto out;

		ret_val = e1000_init_nvm_params(hw);
		if (ret_val)
			goto out;

		ret_val = e1000_init_phy_params(hw);
		if (ret_val)
			goto out;

	}

out:
	return (ret_val);
}

/*
 * e1000_get_bus_info - Obtain bus information for adapter
 * @hw: pointer to the HW structure
 *
 * This will obtain information about the HW bus for which the
 * adapter is attached and stores it in the hw structure. This is a
 * function pointer entry point called by drivers.
 */
s32
e1000_get_bus_info(struct e1000_hw *hw)
{
	if (hw->mac.ops.get_bus_info)
		return (hw->mac.ops.get_bus_info(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_clear_vfta - Clear VLAN filter table
 * @hw: pointer to the HW structure
 *
 * This clears the VLAN filter table on the adapter. This is a function
 * pointer entry point called by drivers.
 */
void
e1000_clear_vfta(struct e1000_hw *hw)
{
	if (hw->mac.ops.clear_vfta)
		hw->mac.ops.clear_vfta(hw);
}

/*
 * e1000_write_vfta - Write value to VLAN filter table
 * @hw: pointer to the HW structure
 * @offset: the 32-bit offset in which to write the value to.
 * @value: the 32-bit value to write at location offset.
 *
 * This writes a 32-bit value to a 32-bit offset in the VLAN filter
 * table. This is a function pointer entry point called by drivers.
 */
void
e1000_write_vfta(struct e1000_hw *hw, u32 offset, u32 value)
{
	if (hw->mac.ops.write_vfta)
		hw->mac.ops.write_vfta(hw, offset, value);
}

/*
 * e1000_update_mc_addr_list - Update Multicast addresses
 * @hw: pointer to the HW structure
 * @mc_addr_list: array of multicast addresses to program
 * @mc_addr_count: number of multicast addresses to program
 *
 * Updates the Multicast Table Array.
 * The caller must have a packed mc_addr_list of multicast addresses.
 */
void
e1000_update_mc_addr_list(struct e1000_hw *hw, u8 *mc_addr_list,
    u32 mc_addr_count)
{
	if (hw->mac.ops.update_mc_addr_list)
		hw->mac.ops.update_mc_addr_list(hw,
		    mc_addr_list, mc_addr_count);
}

/*
 * e1000_force_mac_fc - Force MAC flow control
 * @hw: pointer to the HW structure
 *
 * Force the MAC's flow control settings. Currently no func pointer exists
 * and all implementations are handled in the generic version of this
 * function.
 */
s32
e1000_force_mac_fc(struct e1000_hw *hw)
{
	return (e1000_force_mac_fc_generic(hw));
}

/*
 * e1000_check_for_link - Check/Store link connection
 * @hw: pointer to the HW structure
 *
 * This checks the link condition of the adapter and stores the
 * results in the hw->mac structure. This is a function pointer entry
 * point called by drivers.
 */
s32
e1000_check_for_link(struct e1000_hw *hw)
{
	if (hw->mac.ops.check_for_link)
		return (hw->mac.ops.check_for_link(hw));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_check_mng_mode - Check management mode
 * @hw: pointer to the HW structure
 *
 * This checks if the adapter has manageability enabled.
 * This is a function pointer entry point called by drivers.
 */
bool
e1000_check_mng_mode(struct e1000_hw *hw)
{
	if (hw->mac.ops.check_mng_mode)
		return (hw->mac.ops.check_mng_mode(hw));

	return (false);
}

/*
 * e1000_mng_write_dhcp_info - Writes DHCP info to host interface
 * @hw: pointer to the HW structure
 * @buffer: pointer to the host interface
 * @length: size of the buffer
 *
 * Writes the DHCP information to the host interface.
 */
s32
e1000_mng_write_dhcp_info(struct e1000_hw *hw, u8 *buffer, u16 length)
{
	return (e1000_mng_write_dhcp_info_generic(hw, buffer, length));
}

/*
 * e1000_reset_hw - Reset hardware
 * @hw: pointer to the HW structure
 *
 * This resets the hardware into a known state. This is a function pointer
 * entry point called by drivers.
 */
s32
e1000_reset_hw(struct e1000_hw *hw)
{
	if (hw->mac.ops.reset_hw)
		return (hw->mac.ops.reset_hw(hw));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_init_hw - Initialize hardware
 * @hw: pointer to the HW structure
 *
 * This inits the hardware readying it for operation. This is a function
 * pointer entry point called by drivers.
 */
s32
e1000_init_hw(struct e1000_hw *hw)
{
	if (hw->mac.ops.init_hw)
		return (hw->mac.ops.init_hw(hw));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_setup_link - Configures link and flow control
 * @hw: pointer to the HW structure
 *
 * This configures link and flow control settings for the adapter. This
 * is a function pointer entry point called by drivers. While modules can
 * also call this, they probably call their own version of this function.
 */
s32
e1000_setup_link(struct e1000_hw *hw)
{
	if (hw->mac.ops.setup_link)
		return (hw->mac.ops.setup_link(hw));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_get_speed_and_duplex - Returns current speed and duplex
 * @hw: pointer to the HW structure
 * @speed: pointer to a 16-bit value to store the speed
 * @duplex: pointer to a 16-bit value to store the duplex.
 *
 * This returns the speed and duplex of the adapter in the two 'out'
 * variables passed in. This is a function pointer entry point called
 * by drivers.
 */
s32
e1000_get_speed_and_duplex(struct e1000_hw *hw, u16 *speed, u16 *duplex)
{
	if (hw->mac.ops.get_link_up_info)
		return (hw->mac.ops.get_link_up_info(hw, speed, duplex));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_setup_led - Configures SW controllable LED
 * @hw: pointer to the HW structure
 *
 * This prepares the SW controllable LED for use and saves the current state
 * of the LED so it can be later restored. This is a function pointer entry
 * point called by drivers.
 */
s32
e1000_setup_led(struct e1000_hw *hw)
{
	if (hw->mac.ops.setup_led)
		return (hw->mac.ops.setup_led(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_cleanup_led - Restores SW controllable LED
 * @hw: pointer to the HW structure
 *
 * This restores the SW controllable LED to the value saved off by
 * e1000_setup_led. This is a function pointer entry point called by drivers.
 */
s32
e1000_cleanup_led(struct e1000_hw *hw)
{
	if (hw->mac.ops.cleanup_led)
		return (hw->mac.ops.cleanup_led(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_blink_led - Blink SW controllable LED
 * @hw: pointer to the HW structure
 *
 * This starts the adapter LED blinking. Request the LED to be setup first
 * and cleaned up after. This is a function pointer entry point called by
 * drivers.
 */
s32
e1000_blink_led(struct e1000_hw *hw)
{
	if (hw->mac.ops.blink_led)
		return (hw->mac.ops.blink_led(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_id_led_init - store LED configurations in SW
 * @hw: pointer to the HW structure
 *
 * Initializes the LED config in SW. This is a function pointer entry point
 * called by drivers.
 */
s32
e1000_id_led_init(struct e1000_hw *hw)
{
	if (hw->mac.ops.id_led_init)
		return (hw->mac.ops.id_led_init(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_led_on - Turn on SW controllable LED
 * @hw: pointer to the HW structure
 *
 * Turns the SW defined LED on. This is a function pointer entry point
 * called by drivers.
 */
s32
e1000_led_on(struct e1000_hw *hw)
{
	if (hw->mac.ops.led_on)
		return (hw->mac.ops.led_on(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_led_off - Turn off SW controllable LED
 * @hw: pointer to the HW structure
 *
 * Turns the SW defined LED off. This is a function pointer entry point
 * called by drivers.
 */
s32
e1000_led_off(struct e1000_hw *hw)
{
	if (hw->mac.ops.led_off)
		return (hw->mac.ops.led_off(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_reset_adaptive - Reset adaptive IFS
 * @hw: pointer to the HW structure
 *
 * Resets the adaptive IFS. Currently no func pointer exists and all
 * implementations are handled in the generic version of this function.
 */
void
e1000_reset_adaptive(struct e1000_hw *hw)
{
	e1000_reset_adaptive_generic(hw);
}

/*
 * e1000_update_adaptive - Update adaptive IFS
 * @hw: pointer to the HW structure
 *
 * Updates adapter IFS. Currently no func pointer exists and all
 * implementations are handled in the generic version of this function.
 */
void
e1000_update_adaptive(struct e1000_hw *hw)
{
	e1000_update_adaptive_generic(hw);
}

/*
 * e1000_disable_pcie_master - Disable PCI-Express master access
 * @hw: pointer to the HW structure
 *
 * Disables PCI-Express master access and verifies there are no pending
 * requests. Currently no func pointer exists and all implementations are
 * handled in the generic version of this function.
 */
s32
e1000_disable_pcie_master(struct e1000_hw *hw)
{
	return (e1000_disable_pcie_master_generic(hw));
}

/*
 * e1000_config_collision_dist - Configure collision distance
 * @hw: pointer to the HW structure
 *
 * Configures the collision distance to the default value and is used
 * during link setup.
 */
void
e1000_config_collision_dist(struct e1000_hw *hw)
{
	if (hw->mac.ops.config_collision_dist)
		hw->mac.ops.config_collision_dist(hw);
}

/*
 * e1000_rar_set - Sets a receive address register
 * @hw: pointer to the HW structure
 * @addr: address to set the RAR to
 * @index: the RAR to set
 *
 * Sets a Receive Address Register (RAR) to the specified address.
 */
void
e1000_rar_set(struct e1000_hw *hw, u8 *addr, u32 index)
{
	if (hw->mac.ops.rar_set)
		hw->mac.ops.rar_set(hw, addr, index);
}

/*
 * e1000_validate_mdi_setting - Ensures valid MDI/MDIX SW state
 * @hw: pointer to the HW structure
 *
 * Ensures that the MDI/MDIX SW state is valid.
 */
s32
e1000_validate_mdi_setting(struct e1000_hw *hw)
{
	if (hw->mac.ops.validate_mdi_setting)
		return (hw->mac.ops.validate_mdi_setting(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_mta_set - Sets multicast table bit
 * @hw: pointer to the HW structure
 * @hash_value: Multicast hash value.
 *
 * This sets the bit in the multicast table corresponding to the
 * hash value.  This is a function pointer entry point called by drivers.
 */
void
e1000_mta_set(struct e1000_hw *hw, u32 hash_value)
{
	if (hw->mac.ops.mta_set)
		hw->mac.ops.mta_set(hw, hash_value);
}

/*
 * e1000_hash_mc_addr - Determines address location in multicast table
 * @hw: pointer to the HW structure
 * @mc_addr: Multicast address to hash.
 *
 * This hashes an address to determine its location in the multicast
 * table. Currently no func pointer exists and all implementations
 * are handled in the generic version of this function.
 */
u32
e1000_hash_mc_addr(struct e1000_hw *hw, u8 *mc_addr)
{
	return (e1000_hash_mc_addr_generic(hw, mc_addr));
}

/*
 * e1000_enable_tx_pkt_filtering - Enable packet filtering on TX
 * @hw: pointer to the HW structure
 *
 * Enables packet filtering on transmit packets if manageability is enabled
 * and host interface is enabled.
 * Currently no func pointer exists and all implementations are handled in the
 * generic version of this function.
 */
bool
e1000_enable_tx_pkt_filtering(struct e1000_hw *hw)
{
	return (e1000_enable_tx_pkt_filtering_generic(hw));
}

/*
 * e1000_mng_host_if_write - Writes to the manageability host interface
 * @hw: pointer to the HW structure
 * @buffer: pointer to the host interface buffer
 * @length: size of the buffer
 * @offset: location in the buffer to write to
 * @sum: sum of the data (not checksum)
 *
 * This function writes the buffer content at the offset given on the host if.
 * It also does alignment considerations to do the writes in most efficient
 * way.  Also fills up the sum of the buffer in *buffer parameter.
 */
s32
e1000_mng_host_if_write(struct e1000_hw *hw, u8 *buffer, u16 length,
    u16 offset, u8 *sum)
{
	if (hw->mac.ops.mng_host_if_write)
		return (hw->mac.ops.mng_host_if_write(hw, buffer, length,
		    offset, sum));

	return (E1000_NOT_IMPLEMENTED);
}

/*
 * e1000_mng_write_cmd_header - Writes manageability command header
 * @hw: pointer to the HW structure
 * @hdr: pointer to the host interface command header
 *
 * Writes the command header after does the checksum calculation.
 */
s32
e1000_mng_write_cmd_header(struct e1000_hw *hw,
    struct e1000_host_mng_command_header *hdr)
{
	if (hw->mac.ops.mng_write_cmd_header)
		return (hw->mac.ops.mng_write_cmd_header(hw, hdr));

	return (E1000_NOT_IMPLEMENTED);
}

/*
 * e1000_mng_enable_host_if - Checks host interface is enabled
 * @hw: pointer to the HW structure
 *
 * Returns E1000_success upon success, else E1000_ERR_HOST_INTERFACE_COMMAND
 *
 * This function checks whether the HOST IF is enabled for command operation
 * and also checks whether the previous command is completed.  It busy waits
 * in case of previous command is not completed.
 */
s32
e1000_mng_enable_host_if(struct e1000_hw *hw)
{
	if (hw->mac.ops.mng_enable_host_if)
		return (hw->mac.ops.mng_enable_host_if(hw));

	return (E1000_NOT_IMPLEMENTED);
}

/*
 * e1000_wait_autoneg - Waits for autonegotiation completion
 * @hw: pointer to the HW structure
 *
 * Waits for autoneg to complete. Currently no func pointer exists and all
 * implementations are handled in the generic version of this function.
 */
s32
e1000_wait_autoneg(struct e1000_hw *hw)
{
	if (hw->mac.ops.wait_autoneg)
		return (hw->mac.ops.wait_autoneg(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_check_reset_block - Verifies PHY can be reset
 * @hw: pointer to the HW structure
 *
 * Checks if the PHY is in a state that can be reset or if manageability
 * has it tied up. This is a function pointer entry point called by drivers.
 */
s32
e1000_check_reset_block(struct e1000_hw *hw)
{
	if (hw->phy.ops.check_reset_block)
		return (hw->phy.ops.check_reset_block(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_read_phy_reg - Reads PHY register
 * @hw: pointer to the HW structure
 * @offset: the register to read
 * @data: the buffer to store the 16-bit read.
 *
 * Reads the PHY register and returns the value in data.
 * This is a function pointer entry point called by drivers.
 */
s32
e1000_read_phy_reg(struct e1000_hw *hw, u32 offset, u16 *data)
{
	if (hw->phy.ops.read_reg)
		return (hw->phy.ops.read_reg(hw, offset, data));

	return (E1000_SUCCESS);
}

/*
 * e1000_write_phy_reg - Writes PHY register
 * @hw: pointer to the HW structure
 * @offset: the register to write
 * @data: the value to write.
 *
 * Writes the PHY register at offset with the value in data.
 * This is a function pointer entry point called by drivers.
 */
s32
e1000_write_phy_reg(struct e1000_hw *hw, u32 offset, u16 data)
{
	if (hw->phy.ops.write_reg)
		return (hw->phy.ops.write_reg(hw, offset, data));

	return (E1000_SUCCESS);
}

/*
 * e1000_release_phy - Generic release PHY
 * @hw: pointer to the HW structure
 *
 * Return if silicon family does not require a semaphore when accessing the
 * PHY.
 */
void
e1000_release_phy(struct e1000_hw *hw)
{
	if (hw->phy.ops.release)
		hw->phy.ops.release(hw);
}

/*
 * e1000_acquire_phy - Generic acquire PHY
 * @hw: pointer to the HW structure
 *
 * Return success if silicon family does not require a semaphore when
 * accessing the PHY.
 */
s32
e1000_acquire_phy(struct e1000_hw *hw)
{
	if (hw->phy.ops.acquire)
		return (hw->phy.ops.acquire(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_read_kmrn_reg - Reads register using Kumeran interface
 * @hw: pointer to the HW structure
 * @offset: the register to read
 * @data: the location to store the 16-bit value read.
 *
 * Reads a register out of the Kumeran interface. Currently no func pointer
 * exists and all implementations are handled in the generic version of
 * this function.
 */
s32
e1000_read_kmrn_reg(struct e1000_hw *hw, u32 offset, u16 *data)
{
	return (e1000_read_kmrn_reg_generic(hw, offset, data));
}

/*
 * e1000_write_kmrn_reg - Writes register using Kumeran interface
 * @hw: pointer to the HW structure
 * @offset: the register to write
 * @data: the value to write.
 *
 * Writes a register to the Kumeran interface. Currently no func pointer
 * exists and all implementations are handled in the generic version of
 * this function.
 */
s32
e1000_write_kmrn_reg(struct e1000_hw *hw, u32 offset, u16 data)
{
	return (e1000_write_kmrn_reg_generic(hw, offset, data));
}

/*
 * e1000_get_cable_length - Retrieves cable length estimation
 * @hw: pointer to the HW structure
 *
 * This function estimates the cable length and stores them in
 * hw->phy.min_length and hw->phy.max_length. This is a function pointer
 * entry point called by drivers.
 */
s32
e1000_get_cable_length(struct e1000_hw *hw)
{
	if (hw->phy.ops.get_cable_length)
		return (hw->phy.ops.get_cable_length(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_get_phy_info - Retrieves PHY information from registers
 * @hw: pointer to the HW structure
 *
 * This function gets some information from various PHY registers and
 * populates hw->phy values with it. This is a function pointer entry
 * point called by drivers.
 */
s32
e1000_get_phy_info(struct e1000_hw *hw)
{
	if (hw->phy.ops.get_info)
		return (hw->phy.ops.get_info(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_phy_hw_reset - Hard PHY reset
 * @hw: pointer to the HW structure
 *
 * Performs a hard PHY reset. This is a function pointer entry point called
 * by drivers.
 */
s32
e1000_phy_hw_reset(struct e1000_hw *hw)
{
	if (hw->phy.ops.reset)
		return (hw->phy.ops.reset(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_phy_commit - Soft PHY reset
 * @hw: pointer to the HW structure
 *
 * Performs a soft PHY reset on those that apply. This is a function pointer
 * entry point called by drivers.
 */
s32
e1000_phy_commit(struct e1000_hw *hw)
{
	if (hw->phy.ops.commit)
		return (hw->phy.ops.commit(hw));

	return (E1000_SUCCESS);
}

/*
 * e1000_set_d0_lplu_state - Sets low power link up state for D0
 * @hw: pointer to the HW structure
 * @active: boolean used to enable/disable lplu
 *
 * Success returns 0, Failure returns 1
 *
 * The low power link up (lplu) state is set to the power management level D0
 * and SmartSpeed is disabled when active is true, else clear lplu for D0
 * and enable Smartspeed.  LPLU and Smartspeed are mutually exclusive.  LPLU
 * is used during Dx states where the power conservation is most important.
 * During driver activity, SmartSpeed should be enabled so performance is
 * maintained.  This is a function pointer entry point called by drivers.
 */
s32
e1000_set_d0_lplu_state(struct e1000_hw *hw, bool active)
{
	if (hw->phy.ops.set_d0_lplu_state)
		return (hw->phy.ops.set_d0_lplu_state(hw, active));

	return (E1000_SUCCESS);
}

/*
 * e1000_set_d3_lplu_state - Sets low power link up state for D3
 * @hw: pointer to the HW structure
 * @active: boolean used to enable/disable lplu
 *
 * Success returns 0, Failure returns 1
 *
 * The low power link up (lplu) state is set to the power management level D3
 * and SmartSpeed is disabled when active is true, else clear lplu for D3
 * and enable Smartspeed.  LPLU and Smartspeed are mutually exclusive.  LPLU
 * is used during Dx states where the power conservation is most important.
 * During driver activity, SmartSpeed should be enabled so performance is
 * maintained.  This is a function pointer entry point called by drivers.
 */
s32
e1000_set_d3_lplu_state(struct e1000_hw *hw, bool active)
{
	if (hw->phy.ops.set_d3_lplu_state)
		return (hw->phy.ops.set_d3_lplu_state(hw, active));

	return (E1000_SUCCESS);
}

/*
 * e1000_read_mac_addr - Reads MAC address
 * @hw: pointer to the HW structure
 *
 * Reads the MAC address out of the adapter and stores it in the HW structure.
 * Currently no func pointer exists and all implementations are handled in the
 * generic version of this function.
 */
s32
e1000_read_mac_addr(struct e1000_hw *hw)
{
	if (hw->mac.ops.read_mac_addr)
		return (hw->mac.ops.read_mac_addr(hw));

	return (e1000_read_mac_addr_generic(hw));
}

/*
 * e1000_read_pba_num - Read device part number
 * @hw: pointer to the HW structure
 * @pba_num: pointer to device part number
 *
 * Reads the product board assembly (PBA) number from the EEPROM and stores
 * the value in pba_num.
 * Currently no func pointer exists and all implementations are handled in the
 * generic version of this function.
 */
s32
e1000_read_pba_num(struct e1000_hw *hw, u32 *pba_num)
{
	return (e1000_read_pba_num_generic(hw, pba_num));
}

/*
 * e1000_validate_nvm_checksum - Verifies NVM (EEPROM) checksum
 * @hw: pointer to the HW structure
 *
 * Validates the NVM checksum is correct. This is a function pointer entry
 * point called by drivers.
 */
s32
e1000_validate_nvm_checksum(struct e1000_hw *hw)
{
	if (hw->nvm.ops.validate)
		return (hw->nvm.ops.validate(hw));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_update_nvm_checksum - Updates NVM (EEPROM) checksum
 * @hw: pointer to the HW structure
 *
 * Updates the NVM checksum. Currently no func pointer exists and all
 * implementations are handled in the generic version of this function.
 */
s32
e1000_update_nvm_checksum(struct e1000_hw *hw)
{
	if (hw->nvm.ops.update)
		return (hw->nvm.ops.update(hw));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_reload_nvm - Reloads EEPROM
 * @hw: pointer to the HW structure
 *
 * Reloads the EEPROM by setting the "Reinitialize from EEPROM" bit in the
 * extended control register.
 */
void
e1000_reload_nvm(struct e1000_hw *hw)
{
	if (hw->nvm.ops.reload)
		hw->nvm.ops.reload(hw);
}

/*
 * e1000_read_nvm - Reads NVM (EEPROM)
 * @hw: pointer to the HW structure
 * @offset: the word offset to read
 * @words: number of 16-bit words to read
 * @data: pointer to the properly sized buffer for the data.
 *
 * Reads 16-bit chunks of data from the NVM (EEPROM). This is a function
 * pointer entry point called by drivers.
 */
s32
e1000_read_nvm(struct e1000_hw *hw, u16 offset, u16 words, u16 *data)
{
	if (hw->nvm.ops.read)
		return (hw->nvm.ops.read(hw, offset, words, data));

	return (-E1000_ERR_CONFIG);
}

/*
 * e1000_write_nvm - Writes to NVM (EEPROM)
 * @hw: pointer to the HW structure
 * @offset: the word offset to read
 * @words: number of 16-bit words to write
 * @data: pointer to the properly sized buffer for the data.
 *
 * Writes 16-bit chunks of data to the NVM (EEPROM). This is a function
 * pointer entry point called by drivers.
 */
s32
e1000_write_nvm(struct e1000_hw *hw, u16 offset, u16 words, u16 *data)
{
	if (hw->nvm.ops.write)
		return (hw->nvm.ops.write(hw, offset, words, data));

	return (E1000_SUCCESS);
}

/*
 * e1000_write_8bit_ctrl_reg - Writes 8bit Control register
 * @hw: pointer to the HW structure
 * @reg: 32bit register offset
 * @offset: the register to write
 * @data: the value to write.
 *
 * Writes the PHY register at offset with the value in data.
 * This is a function pointer entry point called by drivers.
 */
s32
e1000_write_8bit_ctrl_reg(struct e1000_hw *hw, u32 reg, u32 offset, u8 data)
{
	return (e1000_write_8bit_ctrl_reg_generic(hw, reg, offset, data));
}

/*
 * e1000_power_up_phy - Restores link in case of PHY power down
 * @hw: pointer to the HW structure
 *
 * The phy may be powered down to save power, to turn off link when the
 * driver is unloaded, or wake on lan is not enabled (among others).
 */
void
e1000_power_up_phy(struct e1000_hw *hw)
{
	if (hw->phy.ops.power_up)
		hw->phy.ops.power_up(hw);

	(void) e1000_setup_link(hw);
}

/*
 * e1000_power_down_phy - Power down PHY
 * @hw: pointer to the HW structure
 *
 * The phy may be powered down to save power, to turn off link when the
 * driver is unloaded, or wake on lan is not enabled (among others).
 */
void
e1000_power_down_phy(struct e1000_hw *hw)
{
	if (hw->phy.ops.power_down)
		hw->phy.ops.power_down(hw);
}

/*
 * e1000_shutdown_fiber_serdes_link - Remove link during power down
 * @hw: pointer to the HW structure
 *
 * Shutdown the optics and PCS on driver unload.
 */
void
e1000_shutdown_fiber_serdes_link(struct e1000_hw *hw)
{
	if (hw->mac.ops.shutdown_serdes)
		hw->mac.ops.shutdown_serdes(hw);
}
