
+ 下载软件
```bash
	apt update
	apt install hostapd dnsmasq iw
```


+ 下载脚本
```bash
	git clone  https://github.com/Embedfire-imx6/ebf-tools-src.git
	cd ebf-tools-src/create_ap/
	chmod +x create_ap
	
```
+ 配置环境

执行命令update-alternatives --config iptables，输入数字1
```bash
root@npi:~/ebf-tools-src/create_ap# update-alternatives --config iptables
There are 2 choices for the alternative iptables (providing /usr/sbin/iptables).

  Selection    Path                       Priority   Status
------------------------------------------------------------
* 0            /usr/sbin/iptables-nft      20        auto mode
  1            /usr/sbin/iptables-legacy   10        manual mode
  2            /usr/sbin/iptables-nft      20        manual mode

Press <enter> to keep the current choice[*], or type selection number: 1
update-alternatives: using /usr/sbin/iptables-legacy to provide /usr/sbin/iptables (iptables) in manual mode
```
+ 运行脚本

假设wifi接口为wlan0，连接外网的接口是eth1，创建一个热点，名字为npi，密码为123456789，
运行脚本./create_ap wlan0 eth1 npi 123456789 --no-virt，看到如下的终端信息，说明创建成功
```bash
root@npi:~/ebf-tools-src/create_ap# ./create_ap wlan0 eth1 npi 123456789 --no-virt
Config dir: /tmp/create_ap.wlan0.conf.angbAubQ
PID: 1198
command failed: Input/output error (-5)
[ 2030.385168] [dhd-wlan0] wl_android_wifi_on : in g_wifi_on=0
[ 2031.789838] [dhd] dhd_conf_read_config : Ignore config file /lib/firmware/bcm/AP6236/Wi-Fi/config.txt
[ 2031.799704] [dhd] dhd_conf_set_path_params : Final fw_path=/lib/firmware/bcm/AP6236/Wi-Fi/fw_bcm43436b0.bin
[ 2031.811048] [dhd] dhd_conf_set_path_params : Final nv_path=/lib/firmware/bcm/AP6236/Wi-Fi/nvram_ap6236.txt
[ 2031.822085] [dhd] dhd_conf_set_path_params : Final clm_path=/lib/firmware/bcm/AP6236/Wi-Fi/clm_bcm43436b0.blob
[ 2031.833326] [dhd] dhd_conf_set_path_params : Final conf_path=/lib/firmware/bcm/AP6236/Wi-Fi/config.txt
[ 2032.071716] [dhd] dhd_conf_set_txglom_params : txglom_mode=multi-desc
[ 2032.081340] [dhd] dhd_conf_set_txglom_params : txglomsize=36, deferred_tx_len=0
[ 2032.090616] [dhd] dhd_conf_set_txglom_params : txinrx_thres=128, dhd_txminmax=-1
[ 2032.099353] [dhd] dhd_conf_set_txglom_params : tx_max_offset=0, txctl_tmo_fix=300
[ 2032.108213] [dhd] dhd_conf_get_disable_proptx : fw_proptx=1, disable_proptx=-1
[ 2032.195122] [dhd] dhd_conf_set_country : set country CN, revision 0
[ 2032.209100] [dhd] dhd_conf_set_country : Country code: CN (CN/0)
[ 2032.225268] [dhd] CONFIG-ERROR) dhd_conf_set_intiovar : ampdu_mpdu setting failed -29
[ 2032.233274] [dhd-wlan0] wl_android_wifi_on : Success
Sharing Internet using method: nat
hostapd command-line interface: hostapd_cli -p /tmp/create_ap.wlan0.conf.angbAubQ/hostapd_ctrl
Configuration file: /tmp/create_ap.wlan0.conf.angbAubQ/hostapd.conf
rfkill: Cannot open RFKILL control device
[ 2032.744618] [dhd-wlan0] wl_cfg80211_del_station : Disconnect STA : 66:66:3a:66:66:3a scb_val.val 3
Using interface wlan0 with hwaddr 28:ed:e0:75:3c:9e and ssid "npi"
[ 2032.842184] [dhd-wlan0] wl_cfg80211_set_channel : netdev_ifidx(5), chan_type(1) target channel(1)
[ 2032.907914] [dhd-wlan0] wl_cfg80211_bcn_bringup_ap : Creating AP with sec=wpapsk/mfpn/0x46
[ 2032.922593] [dhd-wlan0] wl_iw_event : Link UP with 28:ed:e0:75:3c:9e
[ 2032.929155] [dhd-wlan0] wl_ext_iapsta_event : [A] Link up w/o creating? (etype=16)
[ 2032.946892] [dhd-wlan0] wl_notify_connect_status_ap : AP/GO Link up
wlan0: interface state UNINITIALIZED->ENABLED
wlan0: AP-ENABLED

```
