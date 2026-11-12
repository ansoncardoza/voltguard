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
    std::uint16_t transactionId{};
    std::uint16_t protocolId{};
    std::uint16_t length{};
    Byte unitId{};
    Byte functionCode{};
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

    if (packet.protocolId != 0)
    {
        error = "Invalid Protocol ID.";
        return false;
    }

    if (packet.length != frame.size() - 6)
    {
        error = "MBAP length does not match frame size.";
        return false;
    }

    if (packet.length < 2 || packet.length > 254)
    {
        error = "MBAP length is outside the permitted range.";
        return false;
    }

    packet.data.assign(frame.begin() + 8, frame.end());

    return true;
}

bool validateFunction(
    const ModbusPacket& packet,
    std::string& result)
{
    if (packet.unitId < 1 || packet.unitId > 247)
    {
        result = "INVALID UNIT ID";
        return false;
    }

    if (packet.functionCode == 0x03)
    {
        if (packet.data.size() != 4)
        {
            result = "INVALID DATA LENGTH FOR FUNCTION 0x03";
            return false;
        }

        std::uint16_t startAddress = read16(packet.data, 0);
        std::uint16_t quantity = read16(packet.data, 2);

        if (quantity < 1 || quantity > 125)
        {
            result = "INVALID REGISTER QUANTITY (allowed: 1-125)";
            return false;
        }

        std::uint32_t endAddress =
            static_cast<std::uint32_t>(startAddress) + quantity;

        if (endAddress > 65536)
        {
            result = "REGISTER RANGE EXCEEDS ADDRESS SPACE";
            return false;
        }

        result = "VALID READ HOLDING REGISTERS REQUEST";
        return true;
    }

    if (packet.functionCode == 0x06)
    {
        if (packet.data.size() != 4)
        {
            result = "INVALID DATA LENGTH FOR FUNCTION 0x06";
            return false;
        }

        result = "VALID WRITE SINGLE REGISTER REQUEST";
        return true;
    }

    result = "UNSUPPORTED FUNCTION CODE";
    return false;
}

void printPacket(const ModbusPacket& packet)
{
    std::cout << "Transaction ID : 0x"
              << std::hex << std::uppercase
              << std::setw(4) << std::setfill('0')
              << packet.transactionId << "\n";

    std::cout << "Protocol ID    : 0x"
              << std::setw(4) << packet.protocolId << "\n";

    std::cout << "Length         : "
              << std::dec << packet.length << " bytes\n";

    std::cout << "Unit ID        : "
              << static_cast<int>(packet.unitId) << "\n";

    std::cout << "Function Code  : 0x"
              << std::hex << std::uppercase
              << std::setw(2) << static_cast<int>(packet.functionCode)
              << "\n";

    std::cout << "Data bytes     : "
              << std::dec << packet.data.size() << "\n";

    if (packet.functionCode == 0x03 && packet.data.size() == 4)
    {
        std::cout << "Start Address  : "
                  << read16(packet.data, 0) << "\n";

        std::cout << "Quantity       : "
                  << read16(packet.data, 2) << " registers\n";
    }

    if (packet.functionCode == 0x06 && packet.data.size() == 4)
    {
        std::cout << "Register       : "
                  << read16(packet.data, 0) << "\n";

        std::cout << "Write Value    : "
                  << read16(packet.data, 2) << "\n";
    }
}

// Escape special characters before writing JSON strings.

// Convert the original frame bytes into a readable hexadecimal string.
std::string frameToHex(const Frame& frame)
{
    std::ostringstream hex;

    hex << std::uppercase
        << std::hex
        << std::setfill('0');

    for (std::size_t i = 0; i < frame.size(); ++i)
    {
        if (i > 0)
            hex << ' ';

        hex << std::setw(2)
            << static_cast<unsigned int>(frame[i]);
    }

    return hex.str();
}

std::string jsonEscape(const std::string& value)
{
    std::ostringstream escaped;

    for (unsigned char character : value)
    {
        switch (character)
        {
            case '"':  escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;

            default:
                if (character < 0x20)
                {
                    escaped << "\\u"
                            << std::hex << std::setw(4)
                            << std::setfill('0')
                            << static_cast<int>(character);
                }
                else
                {
                    escaped << character;
                }
        }
    }

    return escaped.str();
}

int main()
{
    const std::string inputFile =
        "data/generated/modbus_traffic.hex";

    const std::string outputFile =
        "data/generated/parser_results.json";

    std::ifstream input(inputFile);

    if (!input)
    {
        std::cerr << "Error: Cannot open " << inputFile << "\n";
        return 1;
    }

    std::ofstream jsonOutput(outputFile);

    if (!jsonOutput)
    {
        std::cerr << "Error: Cannot create " << outputFile << "\n";
        return 1;
    }

    std::cout << "====================================\n";
    std::cout << "       VOLTGUARD SYSTEM             \n";
    std::cout << "   FUNCTION VALIDATION ENGINE       \n";
    std::cout << "====================================\n";

    jsonOutput << "{\n"
               << "  \"schema_version\": \"1.0\",\n"
               << "  \"source\": \"VoltGuard C++ Modbus Parser\",\n"
               << "  \"environment\": \"LOCAL_TEST\",\n"
               << "  \"events\": [\n";

    std::string line;
    int frameNumber = 0;
    bool firstEvent = true;

    while (std::getline(input, line))
    {
        if (line.find_first_not_of(" \t\r\n") ==
            std::string::npos)
            continue;

        ++frameNumber;

        Frame frame;
        ModbusPacket packet{};
        std::string error;
        std::string status;
        bool parsed = false;
        bool functionValid = false;

        std::cout << "\n------------------------------------\n";
        std::cout << "Frame #" << frameNumber << "\n";

        if (!parseHexLine(line, frame))
        {
            status = "MALFORMED_HEX_INPUT";
            error = "Unable to parse hexadecimal input.";
            std::cout << "RESULT: " << status << "\n";
        }
        else if (!parseModbusFrame(frame, packet, error))
        {
            status = "INVALID_FRAME";
            std::cout << "RESULT: " << status << "\n";
            std::cout << "Reason: " << error << "\n";
        }
        else
        {
            parsed = true;
            printPacket(packet);

            functionValid = validateFunction(packet, error);

            if (functionValid)
            {
                status = "VALID";
                std::cout << "RESULT: " << error << "\n";
            }
            else
            {
                status = "VALIDATION_FAILED";
                std::cout << "RESULT: " << status << "\n";
                std::cout << "Reason: " << error << "\n";
            }
        }

        if (!firstEvent)
            jsonOutput << ",\n";

        firstEvent = false;

        jsonOutput << "    {\n"
                   << "      \"frame_number\": " << frameNumber << ",\n"
                   << "      \"status\": \"" << status << "\",\n"
                   << "      \"parsed\": "
                   << (parsed ? "true" : "false") << ",\n"
                   << "      \"function_valid\": "
                   << (functionValid ? "true" : "false") << ",\n"
                   << "      \"reason\": \""
                   << jsonEscape(error) << "\",\n"
                   << "      \"raw_hex\": \""
                   << frameToHex(frame) << "\"";

        if (parsed)
        {
            jsonOutput << ",\n"
                       << "      \"transaction_id\": "
                       << packet.transactionId << ",\n"
                       << "      \"protocol_id\": "
                       << packet.protocolId << ",\n"
                       << "      \"length\": "
                       << packet.length << ",\n"
                       << "      \"unit_id\": "
                       << static_cast<int>(packet.unitId) << ",\n"
                       << "      \"function_code\": "
                       << static_cast<int>(packet.functionCode);

            if (packet.functionCode == 0x03 &&
                packet.data.size() == 4)
            {
                jsonOutput << ",\n"
                           << "      \"start_address\": "
                           << read16(packet.data, 0) << ",\n"
                           << "      \"quantity\": "
                           << read16(packet.data, 2);
            }

            if (packet.functionCode == 0x06 &&
                packet.data.size() == 4)
            {
                jsonOutput << ",\n"
                           << "      \"register_address\": "
                           << read16(packet.data, 0) << ",\n"
                           << "      \"register_value\": "
                           << read16(packet.data, 2);
            }
        }

        jsonOutput << "\n    }";
    }

    jsonOutput << "\n  ],\n"
               << "  \"total_frames\": " << frameNumber << "\n"
               << "}\n";

    jsonOutput.close();

    std::cout << "\n====================================\n";
    std::cout << "Validation completed.\n";
    std::cout << "Total frames processed: "
              << frameNumber << "\n";
    std::cout << "JSON report saved to: "
              << outputFile << "\n";

    return 0;
}