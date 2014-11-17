#include "ndn-ns3.hpp"

namespace ndn {

void
Convert::ToPacket(shared_ptr<Block> block, ns3::Ptr<ns3::Packet> packet)
{
  uint32_t type = block->type();
  if (type == 0x05)
    {
        InterestToPacket(block, packet);
    }
  else if (type == 0x06)
    {
      DataToPacket(block, packet);
    }
    else
      throw UnknownHeaderException();
}

Block&
Convert::FromPacket(ns3::Ptr<ns3::Packet> packet)
{
  Buffer buffer(packet->GetSize());
  packet->CopyData(buffer.buf(), packet->GetSize());
  Block *block = new Block(buffer.buf(), packet->GetSize());
  return *block;
}


void
Convert::InterestToPacket(shared_ptr<Block> block, ns3::Ptr<ns3::Packet> packet)
{
  size_t   headerLength;
  uint8_t *headerBuffer;


  block->parse();
  headerLength = block->size();
  headerBuffer = const_cast<uint8_t*>(block->wire());
  NdnHeader ndnHeader(headerBuffer, headerLength);
  ndnHeader.AddNdnHeader(packet, ndnHeader);
}

void
Convert::DataToPacket(shared_ptr<Block> block, ns3::Ptr<ns3::Packet> packet)
{
  size_t   headerLength;
  size_t   trailerLength;
  uint8_t *buffer, *headerBuffer, *trailerBuffer;

  block->parse();

  headerLength = block->size();
  headerLength -= block->get(tlv::SignatureInfo).size();
  headerLength -= block->get(tlv::SignatureValue).size();
  headerLength -= block->get(tlv::Content).size();

  trailerLength = block->get(tlv::SignatureInfo).size() + block->get(tlv::SignatureValue).size();

  buffer = const_cast<uint8_t *>(block->wire());
  headerBuffer  = new uint8_t[headerLength];
  trailerBuffer = new uint8_t[trailerLength];

  memcpy((void *) headerBuffer, (void *) buffer, headerLength);
  memcpy((void *) trailerBuffer, (void *) headerBuffer[int((block->get(tlv::Content).size())+headerLength)], trailerLength);

  NdnHeader ndnHeader(headerBuffer, headerLength);
  NdnTrailer ndnTrailer(trailerBuffer, trailerLength);
  ndnHeader.AddNdnHeader(packet , ndnHeader);
  ndnTrailer.AddNdnTrailer(packet, ndnTrailer);
  delete [] headerBuffer;
  delete [] trailerBuffer;
}

}
