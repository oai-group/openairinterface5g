```bash
git clone https://github.com/richardcochran/linuxptp.git
cd linuxptp/
make
sudo make install
```

-   运行时间同步 注意选择直连的网卡

```bash
# 1. 修改linuxptp/configs/gPTP.cfg
neighborPropDelayThresh	800000
修改为较大的值

# 2. 指定端口运行 gptp
ptp4l -f configs/gPTP.cfg -m -i ens1f0	 # (gg)
ptp4l -f configs/gPTP.cfg -m -i ens802f0 # (uestc-S2600WFT)

# 3. 将同步时间设置到 REALTIME
pmc -u -b 0 -t 1 "SET GRANDMASTER_SETTINGS_NP clockClass 248 \
        clockAccuracy 0xfe offsetScaledLogVariance 0xffff \
        currentUtcOffset 37 leap61 0 leap59 0 currentUtcOffsetValid 1 \
        ptpTimescale 1 timeTraceable 1 frequencyTraceable 0 \
        timeSource 0xa0"
phc2sys -s ens1f0 -c CLOCK_REALTIME --step_threshold=1 \
        --transportSpecific=1 -w -m		# (gg)
phc2sys -s ens802f0 -c CLOCK_REALTIME --step_threshold=1 \
        --transportSpecific=1 -w -m		# (uestc-S2600WFT)
```