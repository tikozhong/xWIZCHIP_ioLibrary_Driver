/*
 * S2E_Packet.c
 *
 *  Created on: 2014. 3. 20.
 *      Author: Raphael
 */

#include <stdio.h>
#include <string.h>
#include "ConfigData.h"
//#include "storageHandler.h"

static S2E_Packet s2e_packet;
static uint16_t S2E_romBase;
static int8_t (*S2E_romWrite)(uint16_t addr, const uint8_t *dat, uint16_t len) = NULL;
static int8_t (*S2E_romRead)(uint16_t addr, uint8_t *dat, uint16_t len) = NULL;

S2E_Packet* setup_S2E_Packet(
	uint16_t base,
	int8_t (*romWrite)(uint16_t addr, const uint8_t *dat, uint16_t len),
	int8_t (*romRead)(uint16_t addr, uint8_t *dat, uint16_t len)
){
	S2E_romBase = base;
	S2E_romWrite = romWrite;
	S2E_romRead = S2E_romRead;
}

S2E_Packet* get_S2E_Packet_pointer()
{
	return &s2e_packet;
}

void set_S2E_Packet_to_factory_value()
{
	s2e_packet.packet_size = sizeof(S2E_Packet);	// 133
	s2e_packet.module_type[0] = 0x01;
	s2e_packet.module_type[1] = 0x02;
	s2e_packet.module_type[2] = 0x00;
	memcpy(s2e_packet.module_name, "WIZ550web\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 25);
	s2e_packet.fw_ver[0] = 0;
	s2e_packet.fw_ver[1] = 0;
	s2e_packet.fw_ver[2] = 0;

	s2e_packet.network_info_common.local_ip[0] = 192;
	s2e_packet.network_info_common.local_ip[1] = 168;
	s2e_packet.network_info_common.local_ip[2] = 11;
	s2e_packet.network_info_common.local_ip[3] = 100;
	s2e_packet.network_info_common.gateway[0] = 192;
	s2e_packet.network_info_common.gateway[1] = 168;
	s2e_packet.network_info_common.gateway[2] = 11;
	s2e_packet.network_info_common.gateway[3] = 1;
	s2e_packet.network_info_common.subnet[0] = 255;
	s2e_packet.network_info_common.subnet[1] = 255;
	s2e_packet.network_info_common.subnet[2] = 255;
	s2e_packet.network_info_common.subnet[3] = 0;

	s2e_packet.serial_info[0].baud_rate = 115200;
	s2e_packet.serial_info[0].data_bits = 8;
	s2e_packet.serial_info[0].parity = 0;
	s2e_packet.serial_info[0].stop_bits = 1;
	s2e_packet.serial_info[0].flow_control = 0;

	s2e_packet.serial_info[1].baud_rate = 115200;
	s2e_packet.serial_info[1].data_bits = 8;
	s2e_packet.serial_info[1].parity = 0;
	s2e_packet.serial_info[1].stop_bits = 1;
	s2e_packet.serial_info[1].flow_control = 0;

	memcpy(s2e_packet.options.pw_setting, "WIZnet\0\0\0\0", 10);
	s2e_packet.options.dhcp_use = 0;
	s2e_packet.options.dns_use = 0;
	s2e_packet.options.dns_server_ip[0] = 8;
	s2e_packet.options.dns_server_ip[1] = 8;
	s2e_packet.options.dns_server_ip[2] = 8;
	s2e_packet.options.dns_server_ip[3] = 8;
	memset(s2e_packet.options.dns_domain_name, '\0', 50);
}

//int read_storage(uint8_t isConfig, void *data, uint16_t size){
//	return 0;
//}

//int write_storage(uint8_t isConfig, void *data, uint16_t size){
//	return 0;
//}

static void load_S2E_Packet_from_storage()
{
	uint32_t check0=0, check1=0,i;
	uint8_t buf[4] = {0};
	uint8_t* p;
	// if there is not rom, load default
	if(S2E_romRead == NULL){
		set_S2E_Packet_to_factory_value();
		return;
	}

	S2E_romRead(S2E_romBase, buf, 4);
	check0 = buf[3];	check0<<=8;
	check0|=buf[2];		check0<<=8;
	check0|=buf[1];		check0<<=8;
	check0|=buf[0];
	
	p = (uint8_t*)&s2e_packet;
	S2E_romRead(S2E_romBase+4, p, sizeof(S2E_Packet));
	
	check1 = 0;
	for(i=0;i<sizeof(S2E_Packet);i++){
		check1 += *p++;
	}
	
	if(s2e_packet.packet_size == 0x0000 || s2e_packet.packet_size == 0xFFFF || 
		check1==0 || check1==0xffffffff || (check0!=check1)){
		set_S2E_Packet_to_factory_value();
		save_S2E_Packet_to_storage();
	}

	s2e_packet.fw_ver[0] = 0;
	s2e_packet.fw_ver[1] = 0;
	s2e_packet.fw_ver[2] = 0;
}

static void save_S2E_Packet_to_storage()
{
	uint32_t check1,i;
	uint8_t buf[4] = {0};
	uint8_t* p;
	// if there is not rom, load default
	if(S2E_romWrite == NULL)	return;
	// generate check code, and write to rom
	check1 = 0;
	p = (uint8_t*)&s2e_packet;
	for(i=0;i<sizeof(S2E_Packet);i++)	check1 += *p++;
	buf[0] = check1 & 0xff;	check1>>=8;
	buf[1] = check1 & 0xff;	check1>>=8;
	buf[2] = check1 & 0xff;	check1>>=8;
	buf[3] = check1 & 0xff;	
	S2E_romWrite(S2E_romBase, buf, 4);
	// write payload
	S2E_romWrite(S2E_romBase+4, (uint8_t*)&s2e_packet, sizeof(S2E_Packet));
}



void get_S2E_Packet_value(void *dest, const void *src, uint16_t size)
{
	memcpy(dest, src, size);
}

void set_S2E_Packet_value(void *dest, const void *value, const uint16_t size)
{
	memcpy(dest, value, size);
}

void set_S2E_Packet(wiz_NetInfo *net)
{
	set_S2E_Packet_value(s2e_packet.network_info_common.mac, net->mac, sizeof(s2e_packet.network_info_common.mac));
	set_S2E_Packet_value(s2e_packet.network_info_common.local_ip, net->ip, sizeof(s2e_packet.network_info_common.local_ip));
	set_S2E_Packet_value(s2e_packet.network_info_common.gateway, net->gw, sizeof(s2e_packet.network_info_common.gateway));
	set_S2E_Packet_value(s2e_packet.network_info_common.subnet, net->sn, sizeof(s2e_packet.network_info_common.subnet));
	set_S2E_Packet_value(s2e_packet.options.dns_server_ip, net->dns, sizeof(s2e_packet.options.dns_server_ip));
	if(net->dhcp == NETINFO_STATIC)
		s2e_packet.options.dhcp_use = 0;
	else
		s2e_packet.options.dhcp_use = 1;
}

void get_S2E_Packet(wiz_NetInfo *net)
{
	get_S2E_Packet_value(net->mac, s2e_packet.network_info_common.mac, sizeof(net->mac));
	get_S2E_Packet_value(net->ip, s2e_packet.network_info_common.local_ip, sizeof(net->ip));
	get_S2E_Packet_value(net->gw, s2e_packet.network_info_common.gateway, sizeof(net->gw));
	get_S2E_Packet_value(net->sn, s2e_packet.network_info_common.subnet, sizeof(net->sn));
	get_S2E_Packet_value(net->dns, s2e_packet.options.dns_server_ip, sizeof(net->dns));
	if(s2e_packet.options.dhcp_use)
		net->dhcp = NETINFO_DHCP;
	else
		net->dhcp = NETINFO_STATIC;
}

void display_Net_Info()
{
	wiz_NetInfo gWIZNETINFO;

	ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
	printf(" # MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
	printf(" # IP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
	printf(" # GW: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
	printf(" # SN: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
	printf(" # DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
}

void Mac_Conf()
{
	S2E_Packet *value = get_S2E_Packet_pointer();
	setSHAR(value->network_info_common.mac);
}

void Net_Conf()
{
	S2E_Packet *value = get_S2E_Packet_pointer();
	wiz_NetInfo gWIZNETINFO;

	/* wizchip netconf */
	get_S2E_Packet_value(gWIZNETINFO.mac, value->network_info_common.mac, sizeof(gWIZNETINFO.mac[0]) * 6);
	get_S2E_Packet_value(gWIZNETINFO.ip, value->network_info_common.local_ip, sizeof(gWIZNETINFO.ip[0]) * 4);
	get_S2E_Packet_value(gWIZNETINFO.gw, value->network_info_common.gateway, sizeof(gWIZNETINFO.gw[0]) * 4);
	get_S2E_Packet_value(gWIZNETINFO.sn, value->network_info_common.subnet, sizeof(gWIZNETINFO.sn[0]) * 4);
	get_S2E_Packet_value(gWIZNETINFO.dns, value->options.dns_server_ip, sizeof(gWIZNETINFO.dns));
	if(value->options.dhcp_use)
		gWIZNETINFO.dhcp = NETINFO_DHCP;
	else
		gWIZNETINFO.dhcp = NETINFO_STATIC;

	ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);

//	display_Net_Info();
}

void set_dhcp_mode()
{
	S2E_Packet *value = get_S2E_Packet_pointer();

	value->options.dhcp_use = 1;
}

void set_static_mode()
{
	S2E_Packet *value = get_S2E_Packet_pointer();

	value->options.dhcp_use = 0;
}

void set_mac(uint8_t *mac)
{
	S2E_Packet *value = get_S2E_Packet_pointer();

	memcpy(value->network_info_common.mac, mac, sizeof(value->network_info_common.mac));
}
