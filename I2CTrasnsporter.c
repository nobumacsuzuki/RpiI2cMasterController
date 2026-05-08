#include "I2CTrasnsporter.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#define I2C_M_WR 0

int gI2cFileHandler = FD_INIT_VAL;

void I2cOperation(I2C_OPERATION_INFO* i2cCommand)
{
    if (i2cCommand->Operation == I2C_OPERATION_WRITE)
    {
        I2cOperationWrite(i2cCommand);
    }
    else if (i2cCommand->Operation == I2C_OPERATION_READ)
    {
        I2cOperationRead(i2cCommand);
    }
    else if (i2cCommand->Operation == I2C_OPERATION_READ_AFTER_WRITE)
    {
        I2cOperationWriteRead(i2cCommand);
    }
}

void I2cOperationWrite(I2C_OPERATION_INFO* i2cCommand)
{
#ifdef _DEBUG
    printf("--- I2C write start---\n");
    printf("Address: 0x%2x\n", i2cCommand->Address);
    printf("Operation: 0x%d\n", i2cCommand->Operation);
    printf("Number of bytes to write: %d\n", i2cCommand->WriteBytes);
    printf("payload to write: ");
    for (unsigned int index = 0; index < i2cCommand->WriteBytes; index++)
    {
        printf("0x%2x, ", i2cCommand->WriteBuffer[index]);
    }
    printf("\n");
#endif

    struct i2c_msg i2cMessage = {i2cCommand->Address, I2C_M_WR, i2cCommand->WriteBytes, i2cCommand->WriteBuffer};
    struct i2c_rdwr_ioctl_data i2cPacket = {&i2cMessage, 1};

    if (ioctl(gI2cFileHandler, I2C_RDWR, &i2cPacket) < 0) 
    {
        printf("ERROR: Failed to perform I2C transaction\n");
    }

#ifdef _DEBUG
    printf("--- I2C write end---\n");
#endif
}

void I2cOperationRead(I2C_OPERATION_INFO* i2cCommand)
{
#ifdef _DEBUG
    printf("--- I2C write start---\n");
    printf("Address: 0x%2x\n", i2cCommand->Address);
    printf("Operation: 0x%d\n", i2cCommand->Operation);
    printf("Number of bytes to read: %d\n", i2cCommand->ReadBytes);
#endif

    struct i2c_msg i2cMessage = {i2cCommand->Address, I2C_M_RD, i2cCommand->ReadBytes, i2cCommand->ReadBuffer};
    struct i2c_rdwr_ioctl_data i2cPacket = {&i2cMessage, 1};

    if (ioctl(gI2cFileHandler, I2C_RDWR, &i2cPacket) < 0) 
    {
        printf("ERROR: Failed to perform I2C transaction\n");
    }

#ifdef _DEBUG
    printf("--- I2C write end---\n");
#endif
}


void I2cOperationWriteRead(I2C_OPERATION_INFO* i2cCommand)
{
#ifdef _DEBUG
    printf("--- I2C read start---\n");
    printf("Address: 0x%2x\n", i2cCommand->Address);
    printf("Operation: 0x%d\n", i2cCommand->Operation);
    printf("Number of bytes to write: %d\n", i2cCommand->WriteBytes);
    printf("payload to write: ");
    for (unsigned int index = 0; index < i2cCommand->WriteBytes; index++)
    {
        printf("0x%2x, ", i2cCommand->WriteBuffer[index]);
    }
    printf("\n");
#endif

    struct i2c_msg i2cMessages[2] = 
    {
        { i2cCommand->Address, I2C_M_WR, i2cCommand->WriteBytes, i2cCommand->WriteBuffer },
        { i2cCommand->Address, I2C_M_RD, i2cCommand->ReadBytes, i2cCommand->ReadBuffer }
    };

    struct i2c_rdwr_ioctl_data i2cPacket = { i2cMessages, 2};

    if (ioctl(gI2cFileHandler, I2C_RDWR, &i2cPacket) < 0) 
    {
        printf("ERROR: Failed to perform I2C transaction\n");
    }

#ifdef _DEBUG
    printf("Number of bytes to read: %d\n", i2cCommand->ReadBytes);
    // create dummy return byte
    for (unsigned int index = 0; index < i2cCommand->ReadBytes; index++)
    {
        printf("0x%2x, ", i2cCommand->ReadBuffer[index]);
    }

    printf("\n");
    printf("--- I2C read end---\n");
#endif
}

bool OpenI2cDevice()
{
#ifdef _DEBUG
    printf("--- I2C device open start---\n");
#endif

    gI2cFileHandler = open(I2C_DEVICE, O_RDWR);
    if (gI2cFileHandler < 0)
    {
        printf("Failed to open I2C device\n");
#ifdef _DEBUG
        printf("--- I2C device open end---\n");
#endif
        return false;
    }
#ifdef _DEBUG
    printf("Suceeed to open I2C device, 0x%2x\n", gI2cFileHandler);
    printf("--- I2C device open end---\n");
#endif
    return true;
}

void CloseI2cDevice()
{
#ifdef _DEBUG
    printf("--- I2C device close start---\n");
    printf("Current I2C device hamdle, 0x%2x\n", gI2cFileHandler);
#endif

    if (gI2cFileHandler != FD_INIT_VAL)
    {
        close(gI2cFileHandler);
        gI2cFileHandler = FD_INIT_VAL;

#ifdef _DEBUG
        printf("I2C device is closed\n");
#endif

    }

#ifdef _DEBUG
    printf("--- I2C device close end---\n");
#endif

}
