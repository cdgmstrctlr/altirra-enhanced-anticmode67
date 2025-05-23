//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2023 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along
//	with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef f_AT_ATNETWORKSOCKETS_INTERNAL_WORKER_H
#define f_AT_ATNETWORKSOCKETS_INTERNAL_WORKER_H

#include <vd2/system/vdstl.h>
#include <at/atnetwork/emusocket.h>
#include <at/atnetworksockets/worker.h>

class ATNetSockWorker;
class IATNetTcpStack;
class VDFunctionThunkInfo;

class ATNetSockBridgeHandler final : public vdrefcounted<IATSocketHandler> {
public:
	ATNetSockBridgeHandler(ATNetSockWorker *parent, SOCKET s, IATStreamSocket *s2, uint32 srcIpAddr, uint16 srcPort, uint32 dstIpAddr, uint16 dstPort);

	uint32 GetSrcIpAddr() const { return mSrcIpAddr; }
	uint16 GetSrcPort() const { return mSrcPort; }
	uint32 GetDstIpAddr() const { return mDstIpAddr; }
	uint16 GetDstPort() const { return mDstPort; }
	ATSocketAddress GetHostAddr() const { return mHostAddr; }

	void Shutdown();

	void SetLocalSocket(IATStreamSocket *s2);
	void SetSrcAddress(const ATSocketAddress& addr);

	void OnNativeSocketConnect();
	void OnNativeSocketReadReady();
	void OnNativeSocketWriteReady();
	void OnNativeSocketClose();
	void OnNativeSocketError();

public:
	void OnSocketOpen() override;
	void OnSocketReadReady(uint32 len) override;
	void OnSocketWriteReady(uint32 len) override;
	void OnSocketClose() override;
	void OnSocketError() override;
	
protected:
	void TryCopyToNative();
	void TryCopyFromNative();

	uint32 mSrcIpAddr = 0;
	uint16 mSrcPort = 0;
	uint32 mDstIpAddr = 0;
	uint16 mDstPort = 0;
	ATSocketAddress mHostAddr;
	ATNetSockWorker *mpParent;
	SOCKET mNativeSocket;
	vdrefptr<IATStreamSocket> mpLocalSocket;
	bool mbLocalClosed;
	bool mbNativeConnected;
	bool mbNativeClosed;
	uint32 mLocalReadAvail;
	uint32 mLocalWriteAvail;

	uint32 mRecvBase;
	uint32 mRecvLimit;
	uint32 mSendBase;
	uint32 mSendLimit;

	char mRecvBuf[1024];
	char mSendBuf[1024];
};

class ATNetSockWorker final : public vdrefcounted<IATNetSockWorker>, public IATEmuNetSocketListener, public IATEmuNetUdpSocketListener {
	friend class ATNetSockBridgeHandler;
public:
	ATNetSockWorker();
	~ATNetSockWorker();

	virtual IATEmuNetSocketListener *AsSocketListener() { return this; }
	virtual IATEmuNetUdpSocketListener *AsUdpListener() { return this; }

	bool Init(IATEmuNetUdpStack *udp, IATEmuNetTcpStack *tcp, bool externalAccess, uint32 forwardingAddr, uint16 forwardingPort);
	void Shutdown();

	void ResetAllConnections();

	bool GetHostAddressesForLocalAddress(bool tcp, uint32 srcIpAddr, uint16 srcPort, uint32 dstIpAddr, uint16 dstPort, ATSocketAddress& hostAddr, ATSocketAddress& remoteAddr) const override;

public:
	virtual bool OnSocketIncomingConnection(uint32 srcIpAddr, uint16 srcPort, uint32 dstIpAddr, uint16 dstPort, IATStreamSocket *socket, IATSocketHandler **handler);
	virtual void OnUdpDatagram(const ATEthernetAddr& srcHwAddr, uint32 srcIpAddr, uint16 srcPort, uint32 dstIpAddr, uint16 dstPort, const void *data, uint32 dataLen);

private:
	enum {
		MYWM_TCP_SOCKET = WM_USER,
		MYWM_UDP_SOCKET,
		MYWM_TCP_LISTEN_SOCKET
	};

	SOCKET CreateUdpConnection(uint32 srcIpAddr, uint16 srcPort, uint32 dstIpAddr, uint16 dstPort, bool redirected);
	void DeleteConnection(SOCKET s);

	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void ProcessUdpDatagram(SOCKET s, uint32 srcIpAddr, uint16 srcPort, uint32 dstIpAddr, uint16 dstPort);

	VDFunctionThunkInfo *mpWndThunk = nullptr;
	ATOM mWndClass = 0;
	HWND mhwnd = nullptr;

	IATEmuNetTcpStack *mpTcpStack = nullptr;
	IATEmuNetUdpStack *mpUdpStack = nullptr;
	bool mbAllowExternalAccess = false;

	uint32 mForwardingAddr = 0;
	uint16 mForwardingPort = 0;

	SOCKET mTcpListeningSocket = INVALID_SOCKET;

	typedef vdhashmap<SOCKET, ATNetSockBridgeHandler *> TcpConnections;
	TcpConnections mTcpConnections;

	struct UdpConnection {
		uint32 mSrcIpAddr;
		uint32 mDstIpAddr;
		uint16 mSrcPort;
		uint16 mDstPort;
	};

	struct UdpConnectionHash {
		size_t operator()(const UdpConnection& conn) const {
			return conn.mSrcIpAddr
				+ conn.mDstIpAddr
				+ conn.mSrcPort
				+ conn.mDstPort
				;
		}
	};

	struct UdpConnectionEqual {
		bool operator()(const UdpConnection& x, const UdpConnection& y) const {
			return x.mSrcIpAddr == y.mSrcIpAddr
				&& x.mDstIpAddr == y.mDstIpAddr
				&& x.mSrcPort == y.mSrcPort
				&& x.mDstPort == y.mDstPort
				;
		}
	};

	typedef vdhashmap<UdpConnection, SOCKET, UdpConnectionHash, UdpConnectionEqual> UdpSourceMap;
	UdpSourceMap mUdpSourceMap;

	typedef vdhashmap<SOCKET, UdpConnection> UdpSocketMap;
	UdpSocketMap mUdpSocketMap;
};

#endif
