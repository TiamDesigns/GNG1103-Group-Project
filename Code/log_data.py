import serial
import time

COM_PORT = "COM9"  # Make sure this matches your ESP32's COM port
BAUD_RATE = 9600
RETRY_DELAY = 2  # Seconds before retrying connection
TIMEOUT = 5  # Increased timeout

# Try connecting to the ESP32 until successful
while True:
    try:
        print(f"Trying to connect to {COM_PORT}...")
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=TIMEOUT)
        print(f"✅ Connected to {COM_PORT}")
        break
    except serial.SerialException:
        print(f"⚠️ ERROR: Cannot open {COM_PORT}. Make sure ESP32 is connected and running a program.")
        time.sleep(RETRY_DELAY)

filename = "scale_data.csv"

# Open the file and start logging
with open(filename, "w") as file:
    file.write("Time(ms),Weight(mg)\n")  # CSV header
    print("✅ Logging started. Press Ctrl+C to stop.")

    try:
        while True:
            try:
                line = ser.readline()  # Read raw bytes from Serial
                
                # Decode safely, ignoring bad bytes
                decoded_line = line.decode("utf-8", errors="ignore").strip()

                # Ensure we only log valid sensor data
                if decoded_line.startswith("DATA"):
                    print(decoded_line)  # Show data in terminal
                    file.write(decoded_line.replace("DATA,", "") + "\n")  # Save to CSV
                elif decoded_line == "START":
                    print("🔄 ESP32 Data Stream Started...")
                elif decoded_line:
                    print(f"⚠️ Ignored Unrecognized Data: {decoded_line}")

            except Exception as e:
                print(f"⚠️ Serial Read Error: {e}")
                time.sleep(1)

    except KeyboardInterrupt:
        print("\n🛑 Logging stopped.")
        ser.close()

# Keep script open after stopping
print("\nLogging has stopped.")
input("Press Enter to exit...")
