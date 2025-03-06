# Use a minimal Debian image
FROM debian:latest  

# Install Wine to run Windows applications on Linux
RUN dpkg --add-architecture i386 && apt update && apt install -y wine wine32

# Set working directory inside the container
WORKDIR /app

# Copy everything (exe + assets) into the container
COPY . /app/

# Run the Windows application using Wine
CMD ["wine", "/app/your_program.exe"]
