/*******************************************************************************
* Function Name  :
* Description    : 对GPRMC数据包进行解析，找到经纬度数据
* Input          :
* Output         :
* Return         :
*******************************************************************************/
uint8_t read_gps_data(uint8_t *gps_buf, uint8_t frame_len, uint8_t *weidu, uint8_t *jindu)
 {
     uint8_t *weidu_s = NULL;
     uint8_t *weidu_o = NULL;
     uint8_t *jingdu_o = NULL;
     uint8_t rtn =0;
     //GPRMC,133735.00,A,3949.63893,N,11616.48419,E,0.296,,120116,,,A*79
     weidu_s  = strstr(GPS_Frame_Buf, ",A,");
     weidu_o  = strstr(GPS_Frame_Buf, ",N,");
     jingdu_o = strstr(GPS_Frame_Buf, ",E,");
     if((weidu_s == NULL) || (weidu_o == NULL) ||(jingdu_o == NULL) )
     {
         rtn = FALSE;
     }
     else
     {
          memset(weidu_buf,0,sizeof(weidu_buf));
          memset(jindu_buf,0,sizeof(jindu_buf));
          memcpy(weidu_buf, weidu_s+3, (weidu_o-weidu_s-3));
          memcpy(jindu_buf, weidu_o+3, (jingdu_o-weidu_o-3));
          printf("\r\n---------------------------------------");
          printf("\r\nGet GPS Frame:\r\n%s\r\n", GPS_Frame_Buf);
          printf("\r\n---------------------------------------");
          rtn = TRUE;
     }
    return rtn;
}


/*******************************************************************************
1.中断负责把GPS串口数据保存到GPS_Uart_Rcv_Buf，在合适的地方调用此函数开始解析。
2.调用本函数会自动关闭接收,然后处理，期间的GPS数据可以忽略。
3.在合适的地方打开接收。
*******************************************************************************/
uint8_t get_gps_useful_data(uint8_t *weidu, uint8_t *jingdu)
{
     uint8_t *p_frame_start = NULL;
     uint8_t *p_useful_frame_start = NULL;
     uint8_t *p_useful_frame_end = NULL;
     int16_t frame_len = 0;
     int16_t rcv_buf_data_len = 0;
     int16_t index = 0;
     int16_t frame_start_point = 0;
     int16_t remain_data_len = 0;
     uint8_t rtn =0;

     gps_rcv_enable(FALSE);
     rcv_buf_data_len = GPS_Rev_Buf_Size;
     p_frame_start = &GPS_Uart_Rcv_Buf[0];

    /* data example:
            $GPVTG,,,,,,,,,N*30
            $GPRMC,132234.00,V,,,,,,,120116,,,N*7D
            $GPRMC,133735.00,A,3949.63893,N,11616.48419,E,0.296,,120116,,,A*79
    */
     for(index = 0; index < rcv_buf_data_len; )
     {
         p_frame_start=strstr(GPS_Uart_Rcv_Buf + index, "$GPRMC,"); //1. find start, "$GPRMC,"
         //$GPRMC,132234.00,V,,,,,,,120116,,,N*7D $GPRMC,133735.00,A,3949.63893,N,11616.48419,E,0.296,,120116,,,A*79
         if(p_frame_start)
         {
              index = p_frame_start-GPS_Uart_Rcv_Buf;
              if(index>0)
                {
                    memset(GPS_Uart_Rcv_Buf, 0 , index);
                }

              index = index + 7;
              p_useful_frame_start = strstr(GPS_Uart_Rcv_Buf + index, ",A,");//2 find useful data, ",A,"
              frame_start_point = p_useful_frame_start - p_frame_start;
              if((frame_start_point > 20)||(frame_start_point <= 0))//not find useful data
                {
                    continue;
                }
                frame_start_point = p_useful_frame_start-GPS_Uart_Rcv_Buf;
                p_useful_frame_end = strstr(GPS_Uart_Rcv_Buf + frame_start_point, ",A*");//3 find useful data end, ",A*"
                frame_len = p_useful_frame_end - p_frame_start + 1;
                if((frame_len > GPS_Frame_Buf_Size)||(frame_len <= 50)) // not found frame end
                {
                    continue;
                }

                //$GPRMC,133735.00,A,3949.63893,N,11616.48419,E,0.296,,120116,,,A*79
                frame_start_point = p_frame_start - GPS_Uart_Rcv_Buf + 1;
                memset(GPS_Frame_Buf, 0, GPS_Frame_Buf_Size);
                memcpy(GPS_Frame_Buf, GPS_Uart_Rcv_Buf + frame_start_point , frame_len);
//                 if(check_frame_xor(GPS_Frame_Buf) != TRUE)
//                 {
//                     continue;
//                 }
                //the frame is correct
                rtn = read_gps_data(GPS_Frame_Buf, remain_data_len, weidu, jingdu);
                if(1 == rtn)
                {
                    break; //found weidu,jingdu data
                }
                else
                {
                     continue;
                }
            }
            else
            {
               break;
            }
        }
        g_gps_data_cnt = 0;
        memset(GPS_Uart_Rcv_Buf, 0, GPS_Rev_Buf_Size);
       return rtn;
}

