FROM gcc:latest

RUN apt-get update && apt-get install -y \
	cmake \
	libgtest-dev \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN mkdir build && cd build && cmake .. && make

CMD ["./build/Fresh-Random-Civ-Picker_CPPGUI"]