#include "I2CTrasnsporter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// I2C operation operand
#define OPTION_WRITE "-w"
#define OPTION_READ "-r"
#define OPTION_READ_AFTER_WRITE "-wr"

// function prototype

void PrintHelp();
I2C_OPERATION ParseOption(char* argvOption);
void CleanHeap(unsigned char* payload, unsigned char* readBuffer);

bool IsI2cAddressValid(char* argvI2cAddress);
bool IsPayloadHexadecimal(char* argvPayload);
bool IsReadBytesDecimal(char* argvReadBytes);
void GetPayload(char* argPayload, size_t argPayloadLength, unsigned char* payload);
void PrintPayload(unsigned char writeBytes, unsigned char* payload);
void PrintReadBuffer(unsigned char readBytes, unsigned char* payload);

void PrintArray(char* msg, unsigned char bytes, unsigned char* buffer);

int main(int argc, char* argv[])
{
    // argc shall be min 3, function name, option and I2C slave address
    if (argc < 3)
    {
        PrintHelp();
        return EXIT_FAILURE;
    }
	char* argvOption = argv[1];
    char*argvI2cAddress = argv[2];

    // parse option
    I2C_OPERATION i2cOperation = ParseOption(argvOption);

    if(i2cOperation == I2C_OPERATION_INVALID)
    {
        printf("ERROR: wrong option, %s\n", argvOption);
        PrintHelp();
        return EXIT_FAILURE;        
    }
    
    // check I2C address sanity
    unsigned char i2cAddress = 0;

    if(IsI2cAddressValid(argvI2cAddress))
    {
        i2cAddress = (unsigned char) strtol(argvI2cAddress, NULL, 16);        
    }
    else
    {
        printf("ERROR: I2C address invalid, %s\n", argvI2cAddress);
        PrintHelp();
        return EXIT_FAILURE;        
    }

    printf("option: %s\n", argvOption);
    printf("address: 7'h%2x\n", i2cAddress);

    // set write payload
    unsigned char writeBytes = 0;
    unsigned char* payload = NULL;
    char* argPayload = NULL;
    size_t argPayloadLength = 0;

    if ((i2cOperation == I2C_OPERATION_WRITE) || (i2cOperation == I2C_OPERATION_READ_AFTER_WRITE))
    {
        if((i2cOperation == I2C_OPERATION_WRITE) && (argc == 3))
        {
            // just write address
            writeBytes = 0;
            payload = NULL;
        }
        else if(((i2cOperation == I2C_OPERATION_WRITE) && (argc == 4)) || ((i2cOperation == I2C_OPERATION_READ_AFTER_WRITE) && (argc == 5)))
        {
            // set payload
            argPayload = argv[3];
            if(IsPayloadHexadecimal(argPayload))
            {
                argPayloadLength = strlen(argPayload);
                writeBytes = argPayloadLength / 2;
                payload = (unsigned char*) malloc(sizeof(unsigned char) * writeBytes);
                if (payload == NULL)
                {
                    printf("ERROR: failed to allocate memory for payload\n");
                    PrintHelp();
                    return EXIT_FAILURE;
                }
                GetPayload(argPayload, argPayloadLength, payload);
                PrintArray("Write Payload", writeBytes, payload);
            }
            else
            {
                // failed in sanity check
                printf("ERROR: payload isn't hexadecimal, or length isn't multilple of 2, %s\n", argPayload);
                PrintHelp();
                return EXIT_FAILURE;        
            }
        }
        else
        {
            // wrong option & arg combination
            printf("ERROR: wrong option and argument combination\n");
            PrintHelp();
            return EXIT_FAILURE;        
        }
    }

    // set read buffer
    unsigned char readBytes = 0;
    unsigned char* readBuffer = NULL;
    char* argvReadBytes = NULL;

    if ((i2cOperation == I2C_OPERATION_WRITE))
    {
        // set read buffer empty
        readBytes = 0;
        readBuffer = NULL;
    }
    else if ((i2cOperation == I2C_OPERATION_READ) || (i2cOperation == I2C_OPERATION_READ_AFTER_WRITE))
    {
        if ((i2cOperation == I2C_OPERATION_READ) && (argc == 4))
        {
            argvReadBytes = argv[3];
        }
        else if ((i2cOperation == I2C_OPERATION_READ_AFTER_WRITE) && (argc == 5))
        {
            argvReadBytes = argv[4];
        }
        else
        {
            printf("ERROR: wrong option and argument combination\n");
            PrintHelp();
            CleanHeap(payload, readBuffer);
            return EXIT_FAILURE;        
        }

        if(IsReadBytesDecimal(argvReadBytes))
        {
            // set read byte size and allocate buffer
            readBytes = (unsigned char) strtol(argvReadBytes, NULL, 10);
            if (readBytes == 0)
            {
                // prohibits 0 byte read
                printf("ERROR: read size shall be greater than 0, %d\n", readBytes);
                PrintHelp();
                CleanHeap(payload, readBuffer);
                return EXIT_FAILURE;
            }
            readBuffer = (unsigned char*) malloc(sizeof(unsigned char) * readBytes);
            if (readBuffer == NULL)
            {
                printf("ERROR: failed to allocate memory for read buffer\n");
                CleanHeap(payload, readBuffer);
                return EXIT_FAILURE;
            }
        }
        else
        {
            printf("ERROR: read byte size shall be decimal, %s\n", argvReadBytes);
            PrintHelp();
            CleanHeap(payload, readBuffer);
            return EXIT_FAILURE;        
        }
    }

    // set I2C transporter operation information
    I2C_OPERATION_INFO i2cCommand = {0};
    i2cCommand.Address = (unsigned int)i2cAddress;
    i2cCommand.Operation = i2cOperation;
    i2cCommand.ReadBuffer = readBuffer;
    i2cCommand.ReadBytes = (unsigned int)readBytes;
    i2cCommand.WriteBuffer = payload;
    i2cCommand.WriteBytes = (unsigned int)writeBytes;

    // transmit I2C packet
    if (OpenI2cDevice())
    {    
#ifdef _DEBUG
        printf("Address: %2x\n", i2cAddress);
        printf("I2c Operation: %d\n", i2cOperation);
        printf("Read size: %d bytes\n", readBytes);
        printf("Write size: %d byte\n", writeBytes);
        if(writeBytes > 0)
        {
            PrintArray("Write Payload", writeBytes, payload);
        }
#endif
        
        I2cOperation(&i2cCommand);
    
        CloseI2cDevice();
    
        if ((i2cOperation == I2C_OPERATION_READ) || (i2cOperation == I2C_OPERATION_READ_AFTER_WRITE))
        {
            PrintArray("Read Buffer", readBytes, readBuffer);
        }
    }

    CleanHeap(payload, readBuffer);
    
    return EXIT_SUCCESS;    
}

void CleanHeap(unsigned char* payload, unsigned char* readBuffer)
{
    if (payload != NULL)
    {
        free(payload);
    }
    if (readBuffer != NULL)
    {
        free(readBuffer);
    }
}


void PrintHelp()
{
    printf("I2cMasterController: write, read and write-to-read I2C slave device\n");
    printf("\n");
    printf("syntax\n");
    printf("I2cMasterController -w (7bit address in hex) (payload in hex)\n");
    printf("   write payload to slave device\n");
    printf("   set slave address only with R/W! = 0 if payload is not given\n");
    printf("I2cMasterController -r (7bit address in hex) (n = decimal)\n");
    printf("   read n bytes from slave device, max 255 bytes\n");
    printf("   n shall be greater than 0");
    printf("I2cMasterController -wr (7bit address in hex) (payload in hex) (n = decimal)\n");
    printf("   read n bytes from slave device, after writing payload (read will follow repeated start)\n");
    printf("   n shall be greater than 0");
    printf("\n");
    printf("----------(example)----------\n");
    printf("I2cMasterController -w 6c\n");
    printf("   set slave address 7'h6c with R/W! = 0\n");
    printf("I2cMasterController -w 6c 0a1b\n");
    printf("   write 0x0a1b to slave device 7'h6c\n");
    printf("I2cMasterController -r 6c 0\n");
    printf("   set slave address 7'h6c with R/W! = 1\n");
    printf("I2cMasterController -w 6c 7\n");
    printf("   read 7 bytes from slave address 7'h6c\n");
    printf("I2cMasterController -wr 6c 0a1b 7\n");
    printf("   read 7 bytes from slave address 7'h6c, after writting 0a1b followed by repeated start\n");
    printf("\n");
    printf("----------(caution)----------\n");
    printf("I2C address shall be hexadecimal, equal to or smaller than 7h'7f\n");
    printf("payload shall be hexadecimal, and its length shall be multiple of 2, e.g. set payload '00' instead of '0' to set 0x00\n");
}

I2C_OPERATION ParseOption(char* argvOption)
{
    I2C_OPERATION i2cOperation = I2C_OPERATION_INVALID;

    if(strcmp(argvOption, OPTION_WRITE) == 0)
    {
        i2cOperation = I2C_OPERATION_WRITE;
    }
    else if(strcmp(argvOption, OPTION_READ) == 0)
    {
        i2cOperation = I2C_OPERATION_READ;
    }
    else if(strcmp(argvOption, OPTION_READ_AFTER_WRITE) == 0)
    {
        i2cOperation = I2C_OPERATION_READ_AFTER_WRITE;
    }
    return i2cOperation;
}

bool IsI2cAddressValid(char* argvI2cAddress)
{

    if (argvI2cAddress == NULL)
    {
        return false;
    }

    // I2C address shall be 7-bit hexadecimal, so the length shall be 2
    if (strlen(argvI2cAddress) != 2)
    {
        return false;
    }

    if (!isxdigit((unsigned char)argvI2cAddress[0]) || !isxdigit((unsigned char)argvI2cAddress[1]))
    {
        return false;
    }

    long address = strtol(argvI2cAddress, &endptr, 16);

    // I2C address shall be range between 0x00 and 0x7f
    if (address < 0x00 || address > 0x7f)
    {
        return false;
    }

    return true;
}

bool IsPayloadHexadecimal(char* argvPayload)
{
    if (argvPayload == NULL)
    {
        return false;
    }

    size_t argPayloadLength = strlen(argvPayload);

    // payload must be non-empty and have even length
    if (argPayloadLength == 0 || (argPayloadLength % 2) != 0)
    {
        return false;
    }

    for (size_t index = 0; index < argPayloadLength; ++index)
    {
        if (!isxdigit((unsigned char)argvPayload[index]))
        {
            return false;
        }
    }

    return true;
}

bool IsReadBytesDecimal(char* argvReadBytes)
{
    if (argvReadBytes == NULL)
    {
        return false;
    }

    size_t length = strlen(argvReadBytes);

    if (length == 0)
    {
        return false;
    }

    for (size_t index = 0; index < length; ++index)
    {
        if (!isdigit((unsigned char)argvReadBytes[index]))
        {
            return false;
        }
    }

    return true;
}

void GetPayload(char* argPayload, size_t argPayloadLength, unsigned char* payload)
{
    for (size_t index = 0; index < argPayloadLength; index += 2)
    {
        /* write directly into payload byte (C99 "%2hhx") */
        sscanf(argPayload + index, "%2hhx", &payload[index / 2]);
    }
}

void PrintArray(char* msg, unsigned char bytes, unsigned char* buffer)
{
    printf("%s :", msg);
    for(size_t index = 0; index < bytes; index++)
    {
        printf("0x%2x, ", buffer[index]);
    }
    printf("\n");
}
