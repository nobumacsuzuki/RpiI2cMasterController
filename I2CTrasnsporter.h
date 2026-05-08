#ifndef _I2C_TRASNSPORTER_H_
#define _I2C_TRASNSPORTER_H_

#include <stdbool.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define I2C_DEVICE ("/dev/i2c-1")
#define FD_INIT_VAL (0xFFFFFFFF)

typedef enum _I2C_OPERATION_
{
    I2C_OPERATION_INVALID = 0,
    I2C_OPERATION_WRITE,
    I2C_OPERATION_READ,
    I2C_OPERATION_READ_AFTER_WRITE
} I2C_OPERATION;

typedef struct _I2C_OPERATION_INFO_
{
    unsigned int    Address;
    I2C_OPERATION   Operation;
    unsigned int    ReadBytes;
    unsigned char* ReadBuffer;
    unsigned int    WriteBytes;
    unsigned char* WriteBuffer;
} I2C_OPERATION_INFO;

void I2cOperation(I2C_OPERATION_INFO* i2cCommand);

void I2cOperationWrite(I2C_OPERATION_INFO* i2cCommand);

void I2cOperationRead(I2C_OPERATION_INFO* i2cCommand);

void I2cOperationWriteRead(I2C_OPERATION_INFO* i2cCommand);

bool OpenI2cDevice();

void CloseI2cDevice();

#endif
