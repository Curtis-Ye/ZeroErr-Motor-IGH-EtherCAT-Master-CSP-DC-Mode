#include "ECconfig.h"

uint16_t alias = 0;                 // 从站别名
uint16_t position0 = 0;             // 位置
uint32_t vendor_id = 0x5a65726f;    // 零售商ID
uint32_t product_code = 0x00029252; // 制造商ID

ec_pdo_entry_info_t slave_0_pdo_entries[] =
    {
        {0x6040, 0x00, 16}, /* Controlword */
        {0x607a, 0x00, 32}, /* Target Position */
        {0x60FF, 0x00, 32}, /* Target Velocity */
        {0x6071, 0x00, 16}, /* Target Torque */

        {0x6041, 0x00, 16}, /* Statusword */
        {0x6064, 0x00, 32}, /* Position Actual Value */
        {0x606c, 0x00, 32}, /* Velocity Actual Value */
        {0x6077, 0x00, 16}, /* Torque Actual Value */
                            //{0x6060, 0x00, 8}, /* Control operation */
};

ec_pdo_info_t slave_0_pdos[] =
    {
        {0x1600, 4, slave_0_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
        {0x1a00, 4, slave_0_pdo_entries + 4}, /* 2nd Transmit PDO Mapping */
};

ec_sync_info_t slave_0_syncs[] =
    {
        {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
        {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},             // 同步管理器0,1用于保留用途，不用于数据交换
        {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_ENABLE}, // 输出过程数据
        {3, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE}, // 输入过程数据
        {0xFF}};

uint controlword, statusword,
    target_position, actual_position,
    target_velocity, actual_velocity,
    target_torque, actual_torque;

ec_pdo_entry_reg_t domain1_regs[] =
    {
        {0, 0, vendor_id, product_code, 0x6040, 0x00, &controlword},
        {0, 0, vendor_id, product_code, 0x607a, 0x00, &target_position},
        {0, 0, vendor_id, product_code, 0x60FF, 0x00, &target_velocity},
        {0, 0, vendor_id, product_code, 0x6071, 0x00, &target_torque},
        {0, 0, vendor_id, product_code, 0x6041, 0x00, &statusword},
        {0, 0, vendor_id, product_code, 0x6064, 0x00, &actual_position},
        {0, 0, vendor_id, product_code, 0x606c, 0x00, &actual_velocity},
        {0, 0, vendor_id, product_code, 0x6077, 0x00, &actual_torque},
        {}};

uint16_t getDriveState(uint16_t statusWord)
{
        return statusWord & 0x6f; // Mask to get state bits
}

ec_slave_config_t *slaveConfig(master, alias, position0, vendor_id, product_code)
{
        ec_slave_config_t *temp;
        temp = ecrt_master_slave_config(master, alias, position0, vendor_id, product_code);
        return temp;
}
