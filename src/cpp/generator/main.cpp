#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using Byte = std::uint8_t;
using Frame = std::vector<Byte>;

void append16(Frame& frame, std::uint16_t value)
{
    frame.push_back(static_cast<Byte>((value >> 8) & 0xFF));
    frame.push_back(static_cast<Byte>(value & 0xFF));
}

Frame createModbusFrame(
    std::uint16_t transactionId,
    Byte unitId,
    Byte functionCode,
    const Frame& data,
    bool malformedLength = false)
{
    Frame frame;

    append16(frame, transactionId);
    append16(frame, 0x0000);

    std::uint16_t length =
        static_cast<std::uint16_t>(2 + data.size());

    if (malformedLength)
        length += 3;

    append16(frame, length);
    frame.push_back(unitId);
    frame.push_back(functionCode);

    frame.insert(frame.end(), data.begin(), data.end());

    return frame;
}

void printFrame(const Frame& frame, const std::string& label)
{
    std::cout << "\n[" << label << "]\n";
    std::cout << "Hex: ";

    for (Byte value : frame)
    {
        std::cout << std::hex << std::uppercase
                  << std::setw(2) << std::setfill('0')
                  << static_cast<int>(value) << " ";
    }

    std::cout << std::dec << "\n";
}

int main(int argc, char* argv[])
{
    std::string mode = "mixed";

    if (argc > 2)
    {
        std::cerr << "Usage: voltguard_generator.exe [normal|mixed]\n";
        return 1;
    }

    if (argc == 2)
    {
        mode = argv[1];

        if (mode != "normal" && mode != "mixed")
        {
            std::cerr << "Error: Invalid mode. Use normal or mixed.\n";
            return 1;
        }
    }

    std::cout << "Traffic mode: " << mode << "\n";

    std::cout << "====================================\n";
    std::cout << "       VOLTGUARD SYSTEM             \n";
    std::cout << "  Modbus/TCP Traffic Generator      \n";
    std::cout << "====================================\n";

    std::vector<Frame> frames;

    // 1. Normal: Read 10 holding registers from address 0.
    frames.push_back(createModbusFrame(
        1, 1, 0x03, {0x00, 0x00, 0x00, 0x0A}));

    // 2. Normal: Write value 1 to holding register 100.
    frames.push_back(createModbusFrame(
        2, 1, 0x06, {0x00, 0x64, 0x00, 0x01}));

    if (mode == "mixed")
    {
        // 3. Unsupported function code 0x45.
        frames.push_back(createModbusFrame(
            3, 1, 0x45, {0x00, 0x00}));

        // 4. Declared length does not match frame.
        frames.push_back(createModbusFrame(
            4, 1, 0x03, {0x00, 0x00, 0x00, 0x02}, true));

        // 5. Invalid FC03 quantity (zero).
        frames.push_back(createModbusFrame(
            5, 1, 0x03, {0x00, 0x00, 0x00, 0x00}));

        // 6. Invalid FC03 quantity (126).
        frames.push_back(createModbusFrame(
            6, 1, 0x03, {0x00, 0x00, 0x00, 0x7E}));

        // 7. FC03 incomplete data.
        frames.push_back(createModbusFrame(
            7, 1, 0x03, {0x00, 0x00, 0x00}));

        // 8. Invalid Unit ID (0 under our test profile).
        frames.push_back(createModbusFrame(
            8, 0, 0x03, {0x00, 0x00, 0x00, 0x0A}));

        // 9. Register range exceeds address space.
        frames.push_back(createModbusFrame(
            9, 1, 0x03, {0xFF, 0xFF, 0x00, 0x02}));
    }

    std::ofstream output(
        "data/generated/modbus_traffic.hex");

    if (!output)
    {
        std::cerr << "Error: Could not create output file.\n";
        return 1;
    }

    for (std::size_t i = 0; i < frames.size(); ++i)
    {
        std::string label =
            (i < 2) ? "NORMAL TRAFFIC" : "ANOMALOUS TEST";

        printFrame(frames[i], label);

        for (Byte value : frames[i])
        {
            output << std::hex << std::uppercase
                   << std::setw(2) << std::setfill('0')
                   << static_cast<int>(value) << " ";
        }

        output << "\n";
    }

    output.close();

    std::cout << "\nGenerated " << frames.size()
              << " synthetic Modbus/TCP frames.\n";

    std::cout << "Saved to: data/generated/modbus_traffic.hex\n";
    std::cout << "Simulation only. No network traffic sent.\n";

    return 0;
}