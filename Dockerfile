# Use a minimal Debian image
FROM debian:latest  

# Install Wine (for running Windows applications on Linux)
RUN dpkg --add-architecture i386 && apt update && apt install -y wine wine32

# Set working directory in the container
WORKDIR /app

# Copy the compiled Windows executable and assets
COPY MyProject/Release/your_program.exe ./
COPY MyProject/assets/ ./assets/

# Run the application using Wine
CMD ["wine", "/app/FRCP_.exe"]
