
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using Byte = std::uint8_t;
using Frame = std::vector<Byte>;

struct ModbusPacket
{
    std::uint16_t transactionId;
    std::uint16_t protocolId;
    std::uint16_t length;
    Byte unitId;
    Byte functionCode;
    std::vector<Byte> data;
};

std::uint16_t read16(const Frame& frame, std::size_t position)
{
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(frame[position]) << 8) |
        frame[position + 1]
    );
}

bool parseHexLine(const std::string& line, Frame& frame)
{
    std::istringstream stream(line);
    std::string token;

    frame.clear();

    while (stream >> token)
    {
        if (token.size() != 2)
            return false;

        try
        {
            std::size_t processed = 0;
            unsigned long value = std::stoul(token, &processed, 16);

            if (processed != token.size() || value > 0xFF)
                return false;

            frame.push_back(static_cast<Byte>(value));
        }
        catch (...)
        {
            return false;
        }
    }

    return !frame.empty();
}

bool parseModbusFrame(
    const Frame& frame,
    ModbusPacket& packet,
    std::string& error)
{
    // MBAP header is 7 bytes; minimum ADU is 8 bytes.
    if (frame.size() < 8)
    {
        error = "Frame is too short.";
        return false;
    }

    packet.transactionId = read16(frame, 0);
    packet.protocolId = read16(frame, 2);
    packet.length = read16(frame, 4);
    packet.unitId = frame[6];
    packet.functionCode = frame[7];

    // Modbus/TCP uses Protocol ID 0.
    if (packet.protocolId != 0)
    {
        error = "Invalid Protocol ID.";
        return false;
    }

    // Length counts Unit ID + PDU.
    if (packet.length != frame.size() - 6)
    {
        error = "MBAP length does not match frame size.";
        return false;
    }

    packet.data.assign(frame.begin() + 8, frame.end());

    return true;
}

bool isSupportedFunction(Byte functionCode)
{
    return functionCode == 0x03 ||
           functionCode == 0x06;
}

void printPacket(const ModbusPacket& packet)
{
    std::cout << "Transaction ID : 0x"
              << std::hex << std::uppercase
              << packet.transactionId << "\n";

    std::cout << "Protocol ID    : 0x"
              << packet.protocolId << "\n";

    std::cout << "Length         : "
              << std::dec << packet.length << " bytes\n";

    std::cout << "Unit ID        : "
              << static_cast<int>(packet.unitId) << "\n";

    std::cout << "Function Code  : 0x"
              << std::hex << std::uppercase
              << static_cast<int>(packet.functionCode) << "\n";

    std::cout << "Data bytes     : "
              << std::dec << packet.data.size() << "\n";
}

int main()
{
    const std::string inputFile =
        "data/generated/modbus_traffic.hex";

    std::ifstream input(inputFile);

    if (!input)
    {
        std::cerr << "Error: Cannot open " << inputFile << "\n";
        return 1;
    }

    std::cout << "====================================\n";
    std::cout << "       VOLTGUARD SYSTEM             \n";
    std::cout << "      MODBUS/TCP PARSER             \n";
    std::cout << "====================================\n";

    std::string line;
    int frameNumber = 0;

    while (std::getline(input, line))
    {
        if (line.find_first_not_of(" \t\r\n") ==
            std::string::npos)
            continue;

        ++frameNumber;

        Frame frame;
        ModbusPacket packet{};
        std::string error;

        std::cout << "\n------------------------------------\n";
        std::cout << "Frame #" << frameNumber << "\n";

        if (!parseHexLine(line, frame))
        {
            std::cout << "RESULT: MALFORMED HEX INPUT\n";
            continue;
        }

        if (!parseModbusFrame(frame, packet, error))
        {
            std::cout << "RESULT: INVALID FRAME\n";
            std::cout << "Reason: " << error << "\n";
            continue;
        }

        printPacket(packet);

        if (!isSupportedFunction(packet.functionCode))
        {
            std::cout << "RESULT: UNSUPPORTED FUNCTION CODE\n";
        }
        else
        {
            std::cout << "RESULT: STRUCTURALLY VALID FRAME\n";
        }
    }

    std::cout << "\n====================================\n";
    std::cout << "Parsing completed.\n";
    std::cout << "Total frames processed: "
              << frameNumber << "\n";

    return 0;
}
