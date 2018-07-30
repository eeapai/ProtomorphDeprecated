#include <stdint.h>
#include <string>

struct SDNSHeader
{
  uint16_t m_wID = 0;
  bool m_bQR = false;
  uint8_t m_byOpCode = 0;
  bool m_bAA = false;
  bool m_bTC = false;
  bool m_bRD = false;
  bool m_bRA = false;
  uint8_t m_byRCODE = 0;
  uint16_t m_wQDCOUNT = 0;
  uint16_t m_wANCOUNT = 0;
  uint16_t m_wNSCOUNT = 0;
  uint16_t m_wARCOUNT = 0;
};

struct SDNSQuestion
{
  std::string m_strQuestion;
  uint16_t m_wQTYPE;
  uint16_t m_wQCLASS;
  bool m_bPreferUnicastResponse;
};

struct SDNSCommonResourceRecord
{
  std::string m_strName;
  uint16_t m_wType;
  uint16_t m_wClass;
  bool m_bMemberOfUniqueRRSet;
  uint32_t m_dwTTL;
  uint16_t m_wResourceDataLength;
};

uint32_t ParseDNSHeader(const uint8_t *pbyData, uint32_t dwAvailableByteCount, SDNSHeader *pPacket);
uint32_t ParseDNSQuestion(const uint8_t *pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, SDNSQuestion *pQuestion);
uint32_t ParseDNSDNSCommonResourceRecord(const uint8_t *pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, SDNSCommonResourceRecord *pCRR);
uint32_t ParseIPv4(const uint8_t *pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, uint8_t *pbyIP);
uint32_t ParseDNSName(const uint8_t *pbyData, uint32_t dwOffset, uint32_t dwAvailableByteCount, std::string &rstrName);
size_t DNSMakeQueryPacket(const char *pcszName, uint8_t *pbyDestination, uint32_t dwMaxLen);

void DNSDumpPacket(const uint8_t *pbyData, uint32_t dwPacketSize, const char *pcszNameFilter = nullptr);
