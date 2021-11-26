#/bin/sh
          enter_reset_detect() {
			n=0
			sleep 1
			while !$(cat /sys/class/gpio/gpio7/value);do
				let 'n+1'
				if [ "$n" -eq 2];then
					sleep 1
					if [ $(cat /sys/class/gpio/gpio7/value) -eq 1 ];then
						reboot -f
					fi
					let 'n+1'
				fi
				
				if [ "$n" -gt 5 ];then
					echo "recovery" > /tmp/recovery
				fi
				sleep 1
			done
	}
	reset_button=$(cat /sys/class/gpio/gpio7/value)                                                                    
        if [ "$reset_button" -eq 0 ];then                                                                                   
				enter_reset_detect																						
        fi
