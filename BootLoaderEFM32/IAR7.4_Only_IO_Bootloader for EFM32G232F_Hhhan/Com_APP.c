
#include "string.h"
#include "Com_APP.h"
#include "system.h"

#define RXBUFF_LENGTH	        255
#define COM_TRANSlENGTH	        128



//#define tr_test
#ifdef tr_test
char r_buf[1024],t_buf[1024];
int r_cnt,t_cnt;
#endif

#define COM_MAX_TRANS_DATA_LENGTH 128

#define REC_SYNC                0xFF
#define SEND_SYNC               0x01

#define COM_BUFF                3

typedef enum
{
    RPacketHead, RWR, RCommd, RLen, RData, RCRC
} REC_STATE;

__no_init static DATA_PACKAGE    Uart_Rx_Package;      //接收数据包
__no_init static DATA_PACKAGE    Uart_Tx_Package;       //发送数据包

__no_init static UCHAR Uart_RxBUff[255];                //接收暂存BUFF
__no_init static UCHAR Uart_TxBUff[255];                //发送暂存BUFF

__no_init static UCHAR by_RxPoint;                      //BUFF位置
__no_init static REC_STATE R_State;                     //接收步骤
__no_init static UCHAR by_TxTimeOut;

int HeaderSize;
unsigned short Current_Transmission_Packet_Size = 0;

__no_init static struct
{
    unsigned RxOk       :1; //接收OK
    unsigned TxOk       :1;
    unsigned StartRx    :1; //传输开始
    unsigned StartTx    :1;
}Com;

void Com_Initial(void)
{
    by_RxPoint = 0;
    Com.RxOk = 0;
    Com.TxOk = 1;
    Com.StartRx = 0;
    R_State = RWR;
    by_TxTimeOut = 0;
    
    memset(&Uart_Rx_Package, 0, sizeof(Uart_Rx_Package));
    memset(&Uart_Tx_Package, 0, sizeof(Uart_Tx_Package));
    memset(&Uart_RxBUff, 0, sizeof(Uart_RxBUff));
    memset(&Uart_TxBUff, 0, sizeof(Uart_TxBUff));
    
    //Initialize the header length of the sending packet
    HeaderSize = sizeof(Uart_Tx_Package) - sizeof(Uart_Tx_Package.Data);
}

//CRC16-CCITT algorithm
unsigned short calculateChecksum(unsigned char* buffer, int size)
{
   uint16_t crc = 0xFFFF;

    if (buffer && size)
        while (size--)
        {
            crc = (crc >> 8) | (crc << 8);
            crc ^= *buffer;
            buffer++;
            crc ^= ((unsigned char) crc) >> 4;
            crc ^= crc << 12;
            crc ^= (crc & 0xFF) << 5;
        }

    return crc;
}

WORD Com_Write(unsigned char readwrite, unsigned short Command, unsigned char Length, unsigned char *pData)
{
    //IF THE UART IS BUSY, WAIT FOR IT TO BECOME IDLE
    while(!Com.TxOk);

    if(Length <= COM_MAX_TRANS_DATA_LENGTH) //MAX ALLOWED DATA BYTE SIZE IS 128
    {
        int current_index = 0;
        Current_Transmission_Packet_Size = 0;
        
        //ADD PACKET HEAD, SEND SYNC BYTE 0xaa55
        Uart_TxBUff[current_index++] = 0x55;
        Uart_TxBUff[current_index++] = 0xaa;
        
        //ADD READ/WRITE INDICATOR
        Uart_TxBUff[current_index++] = readwrite;
        
        //ADD THE COMMAND
        Uart_TxBUff[current_index++] = (UCHAR)Command;
        Uart_TxBUff[current_index++] = (UCHAR)(Command>>8);
        
        //ADD THE LENGTH
        Uart_TxBUff[current_index++] = (UCHAR)Length;
        Uart_TxBUff[current_index++] = (UCHAR)(Length>>8);
        
        //ADD THE DATA to BUFFER
        memcpy(&Uart_TxBUff[current_index], pData, Length);
        current_index += Length;
        
        //CALCULATE THE CHECKSUM AND ADD IT TO THE PACKET
        unsigned short checksum = calculateChecksum(Uart_TxBUff, current_index);
        Uart_TxBUff[current_index++] = checksum;
        Uart_TxBUff[current_index++] = checksum >> 8;
        
        //SET A FLAG THAT ITS NOT OK FOR TRANSMISSION AS IT IS BUSY
        Com.TxOk = 0;
        
        //SET A FLAG TO MENTION TRANSMISSION STARTED
        Com.StartTx = 1;
        
        //SET THE SIZE OF CURRENT PACKET FOR TRANSMISSION
        Current_Transmission_Packet_Size = current_index;
        #ifdef _SP100W_
        HW_Leuart0_EnableTx(1);
        #else
        HW_Com_TX_INT_enable();
        #endif
    }else{
        return ERROR;
    }  
    return Current_Transmission_Packet_Size;
}

//NOT SURE IF THIS IS A LOOPHOLE FOR SOME BUG SOMEDAY
//Since if memcpy is not atomic then, there may be inconsistency in read/write
//as ISR will be writing the received packet structure and the main looping 
//thread copies the value from that structure.
DATA_PACKAGE* Com_GetCommand(DATA_PACKAGE *stData)
{
    DATA_PACKAGE* by_Out = 0;
    if(Com.RxOk)
    {
        memcpy(stData, &Uart_Rx_Package, sizeof(Uart_Rx_Package));       
        memset(&Uart_Rx_Package, 0, sizeof(Uart_Rx_Package));
        Com.RxOk = 0;
        by_Out = stData;
    }
    return by_Out;
}

void Com_10msInt(void)
{
    if(Com.StartTx)     //防止发送超时锁死
    {
        if(by_TxTimeOut++ > 5)    //50ms
        {
            by_TxTimeOut = 0;
            Com.StartTx = 0;
            Com.TxOk = 1;
            memset(Uart_TxBUff,0,sizeof(Uart_TxBUff));                      //清空发送数据BUFF
        }
    }
    else
    {
        by_TxTimeOut = 0;
    }
}

void Com_Tx_Int(void)
{
    static UCHAR nPoint = 0;
    
    if(nPoint < Current_Transmission_Packet_Size){
        #ifdef _SP100W_
        HW_Leuart0_SendData(Uart_TxBUff[nPoint]);
        #else
        HW_Com_Uart_Send_Tx(Uart_TxBUff[nPoint]);
        #endif
      nPoint++;
    }else{
        #ifdef _SP100W_      
        HW_Leuart0_EnableTx(0);
        #else
        HW_Com_TX_INT_disable();
        #endif
        
      #ifdef DEBUG_MS_REPRINTF  
            if(debug_flag)
            printf_h("<--\r\n");  
      #endif 
      nPoint = 0;
      Com.TxOk = 1;
    }
   
}
        
void Com_Rx_Int(UCHAR by_Data)
{
  static unsigned char commandFirstByteReceived = 0;
  static WORD dataLength = 0;
  #ifdef DEBUG_MS_REPRINTF  
  if(debug_flag)
  printf_h("%02X ",by_Data);  
  #endif
    switch(R_State)
    {
    case RPacketHead: //THE PACKET START HAS 0xaa55
      by_RxPoint = 0;
      if(commandFirstByteReceived == 1) //THIS IS THE SECOND BYTE OF THE PACKET HEAD
      {
        if(by_Data == 0xaa) //IF THE SECOND BYTE RECEIVED IS CORRECT 
        {
          Com.StartRx = 1;
          //RECEIVED PACKET HEAD SUCCESSFULLY
          Uart_RxBUff[by_RxPoint++] = 0x55;
          Uart_RxBUff[by_RxPoint++] = 0xaa;
          R_State = RWR;
        }
      }
      else if(by_Data == 0x55)//CAN BE THE PACKET HEAD
      {
        commandFirstByteReceived = 1;
        break; //WAIT FOR THE SECOND BYTE OF PACKET HEAD TO CONFIRM PACKET HEAD
      }      
      commandFirstByteReceived = 0;
      dataLength = 0;
      break;
    case RWR:             //起始位
        if(by_Data == READ_REGISTER || by_Data == WRITE_REGISTER)
        {
          //Com.StartRx = 1;
          Uart_RxBUff[by_RxPoint++] = by_Data;
          R_State = RCommd;
        }
        else
        {
          R_State = RPacketHead;
          //by_RxPoint = 0;
        }
        
        //commandFirstByteReceived = 0;
        //dataLength = 0;
        break;
    case RCommd:            //接收命令
        Uart_RxBUff[by_RxPoint++] = by_Data;
        
        //TOGGLE THIS FLAG ON RECEIVING EACH COMMAND BYTE
        //WHEN THIS FLAG IS 1 THAT MEANS THE FIRST COMMAND BYTE RECEIVED
        //AND WHEN THIS FLAG IS 0 THAT MEANS BOTH COMMAND BYTE RECEIVED
        commandFirstByteReceived = ~commandFirstByteReceived; // == 0) ? 1 : 0;
        
        //WHEN BOTH COMMAND BYTE RECEIVED, MOVE TO NEXT FIELD
        if(commandFirstByteReceived == 0)
          R_State = RLen;        
        break;
    case RLen:      //接收长度

        commandFirstByteReceived = ~commandFirstByteReceived; // == 0) ? 1 : 0;    
        
        Uart_RxBUff[by_RxPoint++] = by_Data;
        
        if(commandFirstByteReceived == 0)
        {
          dataLength = Uart_RxBUff[by_RxPoint-2] + (by_Data << 8);
          
          if(dataLength >= RXBUFF_LENGTH)        //超过最大长度回到同步头
          {
            Com.StartRx = 0;
            by_RxPoint = 0;
            R_State = RPacketHead;
          }
          else if(dataLength == 0)
          {
            R_State = RCRC;
          }
          else
          {
            R_State = RData;
          }
        }
        break;
    case RData:     //接收数据区
        Uart_RxBUff[by_RxPoint++] = by_Data;
        if(--dataLength == 0)
        {
          R_State = RCRC;
        }
        break;
    case RCRC:      //CRC位
        Uart_RxBUff[by_RxPoint++] = by_Data;
        commandFirstByteReceived = ~commandFirstByteReceived;// == 0) ? 1 : 0;
        
        if(commandFirstByteReceived == 0)
        {
          R_State = RPacketHead;
          Com.StartRx = 0;
          Com_RxProcess(by_RxPoint);
          #ifdef DEBUG_MS_REPRINTF  
          if(debug_flag)
            printf_h("-->\r\n");  
          #endif
          by_RxPoint = 0;
        }
        break;
    default:
        R_State = RPacketHead;
        break;
    }
    

}

//uint32_t Com_Rx_Int_Timeout=0;
//uint32_t Com_Rx_OK=1;
//extern uint32_t systimeMs;
//uint8_t   Com_Rx_IntFlag=0;//串口接收中断标志，发生过中断就置1；

//void Com_Rx_Int_TimeoutMode(UCHAR by_Data)
//{
//  Com_Rx_IntFlag=1;
//  Com_Rx_Int_Timeout = systimeMs;
//  #ifdef DEBUG_MS_REPRINTF  
//  if(debug_flag)
//  printf_h("%02X ",by_Data);  
//  #endif
//  if(by_RxPoint>=255){
//     by_RxPoint=0;
//     return;
//  }
//  if(Com_Rx_OK)
//  Uart_RxBUff[by_RxPoint++] = by_Data;
//}
//void Com_Rx_Int_TimeoutCheck(void){
//  if(Com_Rx_IntFlag){
//      if(systimeMs>Com_Rx_Int_Timeout+3){
//          Com_Rx_OK=0;//不要接收数据
//          Com_RxProcess(by_RxPoint);
//          Com_Rx_IntFlag=0;
//          by_RxPoint=0;          
//          Com_Rx_OK=1;//可以接收数据
//          #ifdef DEBUG_MS_REPRINTF  
//          if(debug_flag)
//            printf_h("-->\r\n");  
//          #endif      
//      }
//  }
//  
//}
//收到有效数据处理
void Com_RxProcess(UCHAR receivedPacketLength)
{
    unsigned short crcValue = Uart_RxBUff[receivedPacketLength - 2] + 
      (Uart_RxBUff[receivedPacketLength - 1] << 8);
    unsigned short computedCRC = calculateChecksum(Uart_RxBUff, receivedPacketLength - 2);
    //WHILE CALCULATING THE CHECKSUM, IT SHOULDNOT INCLUDE THE CRC FIELD
    if(crcValue == computedCRC) //CRC校验 
    {
        Uart_Rx_Package.WR = Uart_RxBUff[2];
        Uart_Rx_Package.Command = Uart_RxBUff[3] + (Uart_RxBUff[4] <<8 );
        Uart_Rx_Package.Length  = Uart_RxBUff[5] + (Uart_RxBUff[6] <<8 );
        memcpy(Uart_Rx_Package.Data, &Uart_RxBUff[7], Uart_Rx_Package.Length);  //copy数据
        Com.RxOk = 1;
          #ifdef DEBUG_MS_REPRINTF  
            if(debug_flag)
            printf_h("CRC OK ");  
          #endif        
    }else{
          #ifdef DEBUG_MS_REPRINTF  
            if(debug_flag)
            printf_h("CRC ERROR ");  
          #endif       
          by_RxPoint=0;
    }    
    memset(Uart_RxBUff,0,sizeof(Uart_RxBUff));                      //清空接收数据BUFF
}