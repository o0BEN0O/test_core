#/bin/sh
touch /tmp/core_app.log

Decompress_flag="/tmp/decompress_flag"
Decompress_done_flag="/tmp/decompress_done_flag"
Decompress_fail_flag="/tmp/decompress_fail_flag"

format_p3_flag="/tmp/format_p3_flag"
format_p4_flag="/tmp/format_p4_flag"
format_done_flag="/tmp/format_done"
format_fail_flag="/tmp/format_fail"

DHCP_SERVER_flag="/tmp/DHCP_SERVER_flag"
DHCP_SERVER_done_flag="/tmp/DHCP_SERVER_done_flag"
DHCP_SERVER_fail_flag="/tmp/DHCP_SERVER_fail_flag"

smb_server_flag="/tmp/smb_server_flag"
smb_server_done_flag="/tmp/smb_server_done_flag"
smb_server_fail_flag="/tmp/smb_server_fail_flag"

UPGRADE_FLAG="/tmp/upgrade_flag"

UBOOT_SIZE="/tmp/uboot_size"
UBOOT_OFFS="/tmp/uboot_offs"
UBOOT_FLAG="/tmp/uboot_flag"

SYSTEM_INFO="$(awk  -F "root=" '{print substr($NF,1,14)}' /proc/cmdline)"

upgrade_func() {
	UPGRADE_FILE=$(find /raymarine/Data/ -name "*.upgrade")
	if [ "$SYSTEM_INFO" = "/dev/mmcblk2p3" ]; then
		UBOOT_DEVICE="mmcblk2boot1"		
	fi
	if [ "$SYSTEM_INFO" = "/dev/mmcblk2p4" ]; then
		UBOOT_DEVICE="mmcblk2boot0"		
	fi

	if [ -f "$UBOOT_FLAG" ]; then
		echo 0 > /sys/block/$UBOOT_DEVICE/force_ro
		if [ $? -nq 0 ]; then
			echo "force_ro failed" > /tmp/uboot_flag
			return
		fi
		dd if=$UPGRADE_FILE of=/dev/$UBOOT_DEVICE bs=512 count=$(cat $UBOOT_SIZE) seek=$(cat $UBOOT_OFFS) skip=$(cat $UBOOT_OFFS)
		if [ $? -nq 0 ]; then
			echo "flash u-boot-imx failed !" > /tmp/uboot_flag
			return
		fi
		echo 1 > /sys/block/$UBOOT_DEVICE/force_ro
		echo "upgrade uboot done !" >  /tmp/uboot_flag
	fi

	if [ -f "$BOOTIMG_FLAG" ]; then
		dd if=$UPGRADE_FILE of=/dev/$BOOTIMG_DEVICE bs=512 count=$(cat $BOOTIMG_SIZE) skip=$(cat $BOOTIMG_OFFS)
		if [ $? -nq 0 ]; then
			echo "flash bootimg failed !" > /tmp/bootimg_flag
			return
		fi
		echo "upgrade kernel done !" >  /tmp/bootimg_flag
	fi


}

while true;do
	if [ -f "$DHCP_SERVER_flag" ]; then
		if [ ! -f "/raymarine/Data/dhcpd.conf" ]; then
			cp /etc/dhcp/dhcpd.conf /raymarine/Data/dhcpd.conf && sync
		fi
		killall dhcpd
		dhcpd -f -cf /raymarine/Data/dhcpd.conf -pf /vat/run/dhcpd.pid &
                if [ $? -eq 0 ]; then
                        echo "success"
                        mkfifo $DHCP_SERVER_done_flag
                else
                        echo "fail"
                        mkfifo $DHCP_SERVER_fail_flag
                fi
        fi

        if [ -f "$smb_server_flag" ]; then
                systemctl start smb
		if [ $? -eq 0 ]; then
                        echo "success"
                        mkfifo $smb_server_done_flag
                else
                        echo "fail"
                        mkfifo $smb_server_fail_flag
                fi
        fi

        if [ -f "$Decompress_flag" ]; then
                tar -jxf /raymarine/Data/*.tar.bz2 -C /raymarine/Data
                if [ $? -eq 0 ]; then
                        echo "success"
                        mkfifo $Decompress_done_flag
                else
                        echo "fail"
                        mkfifo $Decompress_fail_flag
                fi
        fi

	if [ -f "$UPGRADE_FLAG" ]; then
		upgrade_func
		rm $UPGRADE_FLAG
	fi

        if [ -p "$format_p3_flag" ]; then
                mkfs.ext4 -F /dev/mmcblk2p3
                if [ $? -eq 0 ]; then
                        echo "success"
                        mkfifo $format_done_flag
                else
                        echo "fail"
                        mkfifo $format_fail_flag
                fi
        fi

        if [ -p "$format_p4_flag" ]; then
                mkfs.ext4 -F /dev/mmcblk2p4
                if [ $? -eq 0 ]; then
                        echo "success"
                        mkfifo $format_done_flag
                else
                        echo "fail"
                        mkfifo $format_fail_flag
                fi
        fi
        sleep 1
done

