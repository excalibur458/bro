// See the file "COPYING" in the main distribution directory for copyright.

#ifndef tunnelhandler_h
#define tunnelhandler_h

#include <netinet/udp.h>
#include "IP.h"
#include "Conn.h"
#include "Sessions.h"
#include "Val.h"


class TunnelInfo {
public:
	TunnelInfo()
		{
		child = 0;
		tunneltype = BifEnum::Tunnel::NONE;
		hdr_len = 0;
		parent.src_addr = parent.dst_addr = 0;
		parent.src_port = parent.dst_port = 0;
		parent.is_one_way = 0;
		}
	~TunnelInfo() 
		{
		if (child) delete child;
		}

	void SetParentIPs(const IP_Hdr *ip_hdr)
		{
		parent.src_addr = ip_hdr->SrcAddr();
		parent.dst_addr = ip_hdr->DstAddr();
		}
	void SetParentPorts(const struct udphdr *uh)
		{
		parent.src_port = uh->uh_sport;
		parent.dst_port = uh->uh_dport;
		}

	RecordVal* GetRecordVal() const 
		{
		RecordVal *rv = new RecordVal(BifType::Record::Tunnel::Parent);
		TransportProto tproto;
		switch(tunneltype) {
		case BifEnum::Tunnel::IP6_IN_IP:
		case BifEnum::Tunnel::IP4_IN_IP:
			tproto = TRANSPORT_UNKNOWN;
			break;
		default:
			tproto = TRANSPORT_UDP;
		} // end switch

		RecordVal* id_val = new RecordVal(conn_id);
		id_val->Assign(0, new AddrVal(parent.src_addr));
		id_val->Assign(1, new PortVal(ntohs(parent.src_port), tproto));
		id_val->Assign(2, new AddrVal(parent.dst_addr));
		id_val->Assign(3, new PortVal(ntohs(parent.dst_port), tproto));
		rv->Assign(0, id_val);
		rv->Assign(1, new EnumVal(tunneltype, BifType::Enum::Tunnel::Tunneltype));
		return rv;
		}

	IP_Hdr *child;
	ConnID parent;
	int hdr_len;
	BifEnum::Tunnel::Tunneltype tunneltype;
};

class TunnelHandler {
public:
	TunnelHandler(NetSessions *arg_s);
	~TunnelHandler();

	// Main entry point. Returns a nil if not tunneled.
	TunnelInfo* DecapsulateTunnel(const IP_Hdr* ip_hdr, int len, int caplen,
			// need those for passing them back to NetSessions::Weird() 
			const struct pcap_pkthdr* hdr, const u_char* const pkt);

protected:
	NetSessions *s;
	short udp_ports[65536]; // which UDP ports to decapsulate
	IP_Hdr* LookForIPHdr(const u_char *data, int datalen);
	TunnelInfo* HandleUDP(const IP_Hdr *ip_hdr, int len, int caplen);
};


#endif