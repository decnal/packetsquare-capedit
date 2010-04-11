/* ethernet.c
 *
 * $Id: ethernet.c 1 2010-04-11 21:04:36 vijay mohan $
 *
 * PacketSquare-capedit - Pcap Edit & Replay Tool
 * By vijay mohan <vijaymohan@packetsquare.com>
 * Copyright 2010 vijay mohan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include<stdint.h>
#include"../main.h"
#include "ethernet.h"
#include<stdlib.h>
#include<string.h>
#include "../packet.h"
#include "../str_to.h"

uint16_t
display_ether(uint8_t **pak)
{
	struct ethhdr *eth_hdr;
	struct vlan_802_1q *vlan_hdr;
	struct mplshdr *mpls_hdr;
	uint16_t i16, i16_2;
	uint32_t i32;
	uint8_t i8;
	uint16_t h_proto;

	eth_hdr = (struct ethhdr *)*pak;
	
	ptree_append("Ethernet II", NULL, STRING, 0, P_ETH_II, 0);
	ptree_append("Source Mac Address", eth_hdr->h_source, MAC, 1, P_ETH_II, 0);
	ptree_append("Destination Mac Address", eth_hdr->h_dest, MAC, 1, P_ETH_II, 0);
	ptree_append("Protocol", &eth_hdr->h_proto, UINT16_HEX, 1, P_ETH_II, 0); 

        cur_pak_info.src_mac = eth_hdr->h_source;
        cur_pak_info.dst_mac = eth_hdr->h_dest;
	
	h_proto = ntohs(eth_hdr->h_proto);

	if (h_proto == 0x8100) {
		*pak += sizeof(struct ethhdr);
		vlan_hdr = (struct vlan_802_1q *)*pak; 
		ptree_append("802.1Q Virtual LAN", NULL, STRING, 0, P_VLAN_802_1Q, 0);
		i16 = *(uint16_t *)vlan_hdr;
		i16_2 = pak_get_bits_uint16(i16, 15, 3);
		ptree_append("Priority", &i16_2, UINT8, 1, P_VLAN_802_1Q, 0);
		i16_2 = pak_get_bits_uint16(i16, 12, 1);
		ptree_append("CFI", &i16_2, UINT8, 1, P_VLAN_802_1Q, 0);
		i16_2 = pak_get_bits_uint16(i16, 11, 12);
		ptree_append("ID", &i16_2, UINT16D, 1, P_VLAN_802_1Q, 0);
		ptree_append("Type", &vlan_hdr->protocol, UINT16_HEX, 1, P_VLAN_802_1Q, 0);

		cur_pak_info.L3_off = sizeof(struct ethhdr) + sizeof(struct vlan_802_1q); 
		*pak += sizeof(struct vlan_802_1q);
		cur_pak_info.L3_proto = ntohs(vlan_hdr->protocol);
		h_proto = cur_pak_info.L3_proto;
	} 
	if (h_proto == 0x8847) {
		if (!(ntohs(eth_hdr->h_proto) == 0x8100)) {
			*pak += sizeof(struct ethhdr);
		}
		mpls_hdr = (struct mplshdr *)*pak;
		ptree_append("Multiprotocol Label Switching Protocol", NULL, STRING, 0, P_MPLS_UNICAST, 0);
		i32 = mpls_hdr->label;
		ptree_append("MPLS Label", &i32, UINT32D, 1, P_MPLS_UNICAST, 0);
		i8  = mpls_hdr->exp;
		ptree_append("MPLS Experimental Bits", &i8, UINT8, 1, P_MPLS_UNICAST, 0);
		i8 = mpls_hdr->stack;
		ptree_append("MPLS Bottom Of Label Stack", &i8, UINT8, 1, P_MPLS_UNICAST, 0);
		i8 = mpls_hdr->ttl;
		ptree_append("MPLS TTL", &i8, UINT8, 1, P_MPLS_UNICAST, 0);

		cur_pak_info.L3_off = sizeof(struct ethhdr) + sizeof(struct mplshdr);
		*pak += sizeof(struct mplshdr);
		cur_pak_info.L3_proto = 0x0800;
	} 
	if ((h_proto == 0x0800) || (h_proto == 0x0806)){
		cur_pak_info.L3_off = sizeof(struct ethhdr);
		*pak += sizeof(struct ethhdr); 
		cur_pak_info.L3_proto = ntohs(eth_hdr->h_proto);
	}
	return cur_pak_info.L3_proto;
}

void
update_ether(char *value)
{
        struct ethhdr *eth_hdr;
	struct vlan_802_1q *vlan_hdr;
	struct mplshdr *mpls_hdr;
        uint16_t proto;
        char cproto[8];
	uint16_t i16;
	uint16_t *p16;

        eth_hdr = (struct ethhdr *)(fpak_curr_info->pak + cur_pak_info.L2_off);
	vlan_hdr = (struct vlan_802_1q *)(fpak_curr_info->pak + cur_pak_info.L2_off + sizeof(struct ethhdr));
	if (ntohs(eth_hdr->h_proto) == 0x8100) {
		mpls_hdr = (struct mplshdr *)(fpak_curr_info->pak + cur_pak_info.L2_off + 
				sizeof(struct ethhdr) + sizeof(struct mplshdr));
	} else {
		mpls_hdr = (struct mplshdr *)(fpak_curr_info->pak + cur_pak_info.L2_off + 
                                sizeof(struct ethhdr));
	}

	i16 = *(uint16_t *)vlan_hdr;
	p16 = (uint16_t *)vlan_hdr;
        if (!strcmp(ptype,"Source Mac Address")) {
		pak_val_update(eth_hdr->h_source, value, MAC);
        } else if (!strcmp(ptype,"Destination Mac Address")) {
		pak_val_update(eth_hdr->h_dest, value, MAC);
        } else if (!strcmp(ptype,"Protocol")) {
		pak_val_update(&eth_hdr->h_proto, value, UINT16_HEX);
        } else if (!strcmp(ptype,"Priority")) {
		*p16 = pak_set_bits_uint16(i16, 15, 3, value);			
	} else if (!strcmp(ptype,"CFI")) {
                *p16 = pak_set_bits_uint16(i16, 12, 1, value);
        } else if (!strcmp(ptype,"ID")) {
                *p16 = pak_set_bits_uint16D(i16, 11, 12, value);
        } else if (!strcmp(ptype,"Type")) {
		pak_val_update(&vlan_hdr->protocol, value, UINT16_HEX);
        } else if (!strcmp(ptype,"MPLS Label")) {
		mpls_hdr->label = atoi(value);
	} else if (!strcmp(ptype,"MPLS Experimental Bits")) {
                mpls_hdr->exp = atoi(value);
        } else if (!strcmp(ptype,"MPLS Bottom Of Label Stack")) {
                mpls_hdr->stack = atoi(value);
        } else if (!strcmp(ptype,"MPLS TTL")) {
                mpls_hdr->ttl = atoi(value);
        }

}

