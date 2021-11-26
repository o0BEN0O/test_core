#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

#define JRD_BIT(nr)                           (1 << (nr))

#define I2C_2 "/dev/i2c-2"

#define KSZ9897_I2C_ADDR 0x5f

#define KSZ9897_PHY_BAS_CTRL_REG(PORT_N) 0x0100|PORT_N<<12
#define KSZ9897_PHY_POWER_BIT	JRD_BIT(3)


/* Ensure address is not busy */
int set_slave_addr(int file, int address, int force)
{
	/* With force, let the user read from/write to the registers
	   even when a driver is also running */
	if (ioctl(file, force ? I2C_SLAVE_FORCE : I2C_SLAVE, address) < 0) {
		fprintf(stderr,
			"Error: Could not set address to 0x%02x: %s\n",
			address, strerror(errno));
		return -errno;
	}

	return 0;
}

/*

	reg_addr: 16 bits register address
	flags: 0 only write
		   1 read register data
	val_buf:
		if flag==0
			val is send data buf
		if flag==1
			val is recv data buf
	cnt:the counter you want to read or write
*/

int i2c_transfer_16bits
(const char *i2c_dev,unsigned char i2c_dev_addr,unsigned long reg_addr,int flags,unsigned char *val_buf,size_t cnt)
{
	int i=0;
	int flag=flags;
	int file=-1;
	int nmsgs=0;
	int nmsgs_sent=0;
	struct i2c_rdwr_ioctl_data rdwr;
	//struct i2c_msg msgs[I2C_RDWR_IOCTL_MAX_MSGS];
	struct i2c_msg msgs[2];
	char *buf;

	for (i = 0; i < 2; i++)
		msgs[i].buf = NULL;

	file=open(i2c_dev,O_RDWR);
	if(file<0){
		printf("open %s failed\n",i2c_dev);
		return -1;
	}

	if(flags==0){
		msgs[nmsgs].addr = i2c_dev_addr;
		msgs[nmsgs].flags = 0;/*write flag*/
		msgs[nmsgs].len = 2+cnt;
		msgs[nmsgs].buf=malloc(msgs[nmsgs].len);/*buf alloc according to len*/
		msgs[nmsgs].buf[0]=(unsigned char)(reg_addr>>8);
		msgs[nmsgs].buf[1]=(unsigned char)(reg_addr&0x00ff);
		if(cnt==0||cnt>2){
			printf("cnt[%d] is error\n",cnt);
			goto err_out;
		}
		if(cnt==1)
			msgs[nmsgs].buf[2]=val_buf[0];
		if(cnt==2){
			msgs[nmsgs].buf[2]=val_buf[0];
			msgs[nmsgs].buf[3]=val_buf[1];
		}
		goto write_to_reg;
	}

	if(flags==1){
		msgs[nmsgs].addr = i2c_dev_addr;
		msgs[nmsgs].flags = 0;/*write flag*/
		msgs[nmsgs].len = 2;
		msgs[nmsgs].buf=malloc(msgs[nmsgs].len);/*buf alloc according to len*/
		msgs[nmsgs].buf[0]=(unsigned char)(reg_addr>>8);
		msgs[nmsgs].buf[1]=(unsigned char)(reg_addr&0x00ff);
	}

	nmsgs++;

/*perpare read buf*/
	msgs[nmsgs].addr = i2c_dev_addr;
	msgs[nmsgs].flags = 1; /*read flag*/
	msgs[nmsgs].len=cnt;
	buf=malloc(msgs[nmsgs].len);/*buf alloc according to len*/
	memset(buf,0,cnt);
	msgs[nmsgs].buf=buf;
/*perpare read buf*/

write_to_reg:
	rdwr.msgs = msgs;
	rdwr.nmsgs = nmsgs+1;/*if read rdwr.nmsgs==2,eles if write rdwr.nmsgs==1*/

#if 1
	int k=0;
	int f=0;

	printf("%s(%d) nmsgs %d\n",__func__,__LINE__,nmsgs);
	for(f=0;f<rdwr.nmsgs;f++){
		printf("%s(%d) addr %d\n",__func__,__LINE__,msgs[f].addr);
		printf("%s(%d) flags %d\n",__func__,__LINE__,msgs[f].flags);
		printf("%s(%d) len %d\n",__func__,__LINE__,msgs[f].len);
		for(k=0;k<msgs[f].len;k++)
			printf("%s(%d) buf 0x%02x \n",__func__,__LINE__,msgs[f].buf[k]);
		printf("\n");
	}
#endif

	nmsgs_sent = ioctl(file, I2C_RDWR, &rdwr);
	if(nmsgs_sent<0)
		goto err_out;
	else if(nmsgs_sent<nmsgs){
		printf("Warning: only %d/%d messages were sent\n", nmsgs_sent, nmsgs);
		goto err_out;
	}

	printf("flag %d cnt %d\n",flag,cnt);

	if(flag==1){
		for(i=0;i<cnt;i++){
			printf("msgs[%d].buf[%d] 0x%02x \n",nmsgs,i,msgs[nmsgs].buf[i]);
			val_buf[i]=msgs[nmsgs].buf[i];
			//printf("val_buf[%d] 0x%02x \n",i,val_buf[i]);
			//memcpy(val_buf+i,msgs[nmsgs].buf+i,1);
		}
	}


	close(file);
	for (i = 0; i <= nmsgs; i++)
		free(msgs[i].buf);

	return 0;


err_out:
	close(file);
	for (i = 0; i <= nmsgs; i++)
		free(msgs[i].buf);
	return -1;
}


void main(void)
{
	unsigned char val_buf=0;
	unsigned long reg_addr=0x4100;
	//memset(val_buf,0,sizeof(val_buf));
	printf("0x%04x\n",KSZ9897_PHY_BAS_CTRL_REG(4));
	i2c_transfer_16bits(I2C_2,KSZ9897_I2C_ADDR,KSZ9897_PHY_BAS_CTRL_REG(4),1,&val_buf,sizeof(val_buf));
	printf("val_buf 0x%02x\n",val_buf);
	val_buf&=(~KSZ9897_PHY_POWER_BIT);
	//val_buf|=KSZ9897_PHY_POWER_BIT;
	i2c_transfer_16bits(I2C_2,KSZ9897_I2C_ADDR,KSZ9897_PHY_BAS_CTRL_REG(4),0,&val_buf,sizeof(val_buf));
	//i2c_transfer_16bits(I2C_2,KSZ9897_I2C_ADDR,reg_addr,0,val_buf,sizeof(val_buf));
	//printf("val 0x%02x  0x%02x \n",val_buf[0],val_buf[1]);
}
