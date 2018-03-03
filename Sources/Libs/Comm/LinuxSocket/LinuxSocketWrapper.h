#ifndef __LINUX_SOCKET_WRAPPER_H__
#define __LINUX_SOCKET_WRAPPER_H__

#include "ICommDevice.h"

class CLinuxSocketWrapper : public ICommDevice
{
public:
  CLinuxSocketWrapper();
  virtual ~CLinuxSocketWrapper() {}
public:
  void StartListening(unsigned short wPort);
  void StopListening();
  void AcceptNewConnection(unsigned long dwTimeout_ms = 100);
  void Connect(const char *pcszDestination, unsigned short wPort);
  void DisconnectClient();
  void Disconnect();

  void ReceiveData(unsigned char *pbyNewData, unsigned long dwNumMaxBytes, unsigned long *pdwNumBytesReceived, unsigned long dwTimeout_ms = 100);
  void SendData(const unsigned char *pbyData, unsigned long dwNumBytes, unsigned long *pdwNumBytesSent);

  bool IsServerOK() const;
  bool IsClientOK() const;

private:
  enum ESocket
  {
    INVALID_SOCKET = -1
  };
  int m_listenSocket = INVALID_SOCKET;
  int m_connectionSocket = INVALID_SOCKET;

public:
  // Inherited via ICommDevice
  int GetStatus() const override;
  void Connect(const char * pcszWhereTo) override;
  void Send(const unsigned char * pbyData, unsigned long dwByteCount, unsigned long * pdwSentByteCount) override;
  void Receive(unsigned char * pbyDestination, unsigned long dwMaxByteCount, unsigned long * pdwHowManyBytes) override;
};

#endif
