/**
 * @author André Furlan <ensismoebius@gmail.com>
 *
 * Based on the original code of:
 * @author Rodrigo Capobianco Guido <guido@ieee.org>
 *
 * This whole project are under GPLv3, for
 * more information read the license file
 *
 * 8 de ago de 2019
 */
#ifndef SRC_WAV_C_
#define SRC_WAV_C_

#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

class Wav {

	private:

		// signal properties
		struct {
				/* RIFF Chunk Descriptor */
				std::uint8_t RIFF[4];        // RIFF Header Magic header
				std::uint32_t chunkSize;      // RIFF Chunk Size
				std::uint8_t WAVE[4];        // WAVE Header

				/* "fmt" sub-chunk */
				std::uint8_t fmt[4];         // FMT header
				std::uint32_t subchunk1Size;  // Size of the fmt chunk
				std::uint16_t audioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
				std::uint16_t numOfChan;      // Number of channels 1=Mono 2=Sterio
				std::uint32_t samplingrate;  // Sampling Frequency in Hz
				std::uint32_t bytesPerSec;    // bytes per second
				std::uint16_t blockAlign;     // 2=16-bit mono, 4=16-bit stereo
				std::uint16_t bitsPerSample;  // Number of bits per sample

				/* "data" sub-chunk */
				std::uint8_t subchunk2ID[4]; // "data"  string
				std::uint32_t subchunk2Size;  // Sampled data length
		} headers;

		// another signal properties
		int amountOfData;
		int waveResolution;

		// signal data
		double* data;
		double* dataLeft;
		double* dataRight;

		// path of file containing the signal
		std::string path;

		// callback function (applied on data)
		void (*callbackFunction)(double* signal, int signalLength, unsigned int samplingRate);

	public:
		Wav() {
			data = 0;
			dataLeft = 0;
			dataRight = 0;
			this->path = path;
		}

		~Wav() {
			if (data != 0) delete[] data;
			if (dataLeft != 0) delete[] dataLeft;
			if (dataRight != 0) delete[] dataRight;
		}

		void process() {

			if (callbackFunction == 0) return;

			int resPlusCha = waveResolution * 10 + this->headers.numOfChan;

			switch (resPlusCha) {
				case 82:
				case 162:
					(*callbackFunction)(dataLeft, amountOfData, this->headers.samplingrate);
					(*callbackFunction)(dataRight, amountOfData, this->headers.samplingrate);
				case 81:
				case 161:
					(*callbackFunction)(data, amountOfData, this->headers.samplingrate);
					break;
				default:
					throw std::runtime_error("Invalid number of channels and/or resolution");
					break;
			}
		}

		void read(std::string path) {

			this->path = path;

			std::ifstream ifs;
			ifs.open(path, std::ios::in | std::ios::binary);
			// Reads the file headers
			readWaveHeaders(ifs);
			// Reads actual data
			readWaveData(ifs);
			ifs.clear();
			ifs.close();
		}

		void write(std::string path) {

			std::ofstream ofs;
			ofs.open(path, std::ios::out | std::ios::binary);

			if (!ofs.is_open()) {
				std::cout << "Cannot open file: " << path;
				throw std::runtime_error("Impossible to open the file!");
				return;
			}

			int resPlusCha = waveResolution * 10 + this->headers.numOfChan;

			switch (resPlusCha) {
				case 81:
					write8Res1Channel(ofs);
					break;
				case 82:
					write8Res2Channel(ofs);
					break;
				case 161:
					write16Res1Channel(ofs);
					break;
				case 162:
					write16Res2Channel(ofs);
				default:
					ofs.close();
					throw std::runtime_error("Invalid number of channels and/or resolution");
					break;
			}

			ofs.close();
		}

		double* getData() const {
			return data;
		}

		double* getDataLeft() const {
			return dataLeft;
		}

		double* getDataRight() const {
			return dataRight;
		}

		std::string getPath() const {
			return path;
		}

		void setCallbackFunction(void (*callbackFunction)(double* signal, int signalLength, unsigned int samplingRate)) {
			this->callbackFunction = callbackFunction;
		}

	private:

		short convert2of8to1of16(unsigned char lsb, unsigned char msb) {
			return (((msb & 0x80) >> 7) * (32768) + ((msb & 0x40) >> 6) * (16384) + ((msb & 0x20) >> 5) * (8192) + ((msb & 0x10) >> 4) * (4096) + ((msb & 0x08) >> 3) * (2048) + ((msb & 0x04) >> 2) * (1024) + ((msb & 0x02) >> 1) * (512) + ((msb & 0x01)) * (256) + ((lsb & 0x80) >> 7) * (128) + ((lsb & 0x40) >> 6) * (64) + ((lsb & 0x20) >> 5) * (32) + ((lsb & 0x10) >> 4) * (16) + ((lsb & 0x08) >> 3) * (8) + ((lsb & 0x04) >> 2) * (4) + ((lsb & 0x02) >> 1) * (2) + (lsb & 0x01));
		}

		void convert1of16to2of8(short result, unsigned char* lsb, unsigned char* msb) {
			*lsb = (((result & 0x0080) >> 7) * (128) + ((result & 0x0040) >> 6) * (64) + ((result & 0x0020) >> 5) * (32) + ((result & 0x0010) >> 4) * (16) + ((result & 0x0008) >> 3) * (8) + ((result & 0x0004) >> 2) * (4) + ((result & 0x0002) >> 1) * (2) + (result & 0x0001));
			*msb = (((result & 0x8000) >> 15) * (128) + ((result & 0x4000) >> 14) * (64) + ((result & 0x2000) >> 13) * (32) + ((result & 0x1000) >> 12) * (16) + ((result & 0x0800) >> 11) * (8) + ((result & 0x0400) >> 10) * (4) + ((result & 0x0200) >> 9) * (2) + ((result & 0x0100) >> 8));
		}

		void readWaveData(std::ifstream& ifs) {

			int resPlusCha = waveResolution * 10 + this->headers.numOfChan;

			switch (resPlusCha) {
				case 81:
					read8Res1Channel(ifs);
					break;
				case 82:
					read8Res2Channel(ifs);
					break;
				case 161:
					read16Res1Channel(ifs);
					break;
				case 162:
					read16Res2Channel(ifs);
				default:
					ifs.close();
					throw std::runtime_error("Invalid number of channels and/or resolution");
					break;
			}
		}

		void readWaveHeaders(std::ifstream& ifs) {

			ifs.seekg(0, std::ios::beg);
			ifs.read((char*) &this->headers, sizeof(this->headers));

			if (this->headers.audioFormat != 1) {
				throw std::runtime_error("Not in PCM format!");
				return;
			}

			waveResolution = (this->headers.bytesPerSec * 8) / (this->headers.numOfChan * this->headers.samplingrate);
			amountOfData = this->headers.chunkSize / this->headers.blockAlign;
		}

		inline void write8Res1Channel(std::ofstream& ofs) {

			ofs.write((char*) (&this->headers), sizeof(this->headers));

			unsigned char waveformdata;

			for (int i = 0; i < amountOfData; i++) {
				waveformdata = (unsigned char) (data[i]);
				ofs.write((char*) (&waveformdata), sizeof(waveformdata));
			}
		}

		inline void write8Res2Channel(std::ofstream& ofs) {

			ofs.write((char*) (&this->headers), sizeof(this->headers));

			unsigned char waveformdata_right, waveformdata_left;

			for (int i = 0; i < amountOfData; i++) {
				waveformdata_left = (unsigned char) (dataLeft[i]);
				ofs.write((char*) (&waveformdata_left), sizeof(waveformdata_left));
				waveformdata_right = (unsigned char) (dataRight[i]);
				ofs.write((char*) (&waveformdata_right), sizeof(waveformdata_right));
			}
		}

		inline void write16Res1Channel(std::ofstream& ofs) {

			ofs.write((char*) (&this->headers), sizeof(this->headers));

			unsigned char waveformdata_lsb, waveformdata_msb;

			for (int i = 0; i < amountOfData; i++) {
				convert1of16to2of8((short) ((data[i])), &waveformdata_lsb, &waveformdata_msb);
				ofs.write((char*) (&waveformdata_lsb), sizeof(waveformdata_lsb));
				ofs.write((char*) (&waveformdata_msb), sizeof(waveformdata_msb));
			}
		}

		inline void write16Res2Channel(std::ofstream& ofs) {

			ofs.write((char*) &this->headers, sizeof(this->headers));

			unsigned char waveformdata_lsb_left, waveformdata_lsb_right, waveformdata_msb_left, waveformdata_msb_right;

			for (int i = 0; i < amountOfData; i++) {
				convert1of16to2of8((short) (dataLeft[i]), &waveformdata_lsb_left, &waveformdata_msb_left);
				convert1of16to2of8((short) (dataRight[i]), &waveformdata_lsb_right, &waveformdata_msb_right);
				ofs.write((char*) (&waveformdata_lsb_left), sizeof(waveformdata_lsb_left));
				ofs.write((char*) (&waveformdata_msb_left), sizeof(waveformdata_msb_left));
				ofs.write((char*) (&waveformdata_lsb_right), sizeof(waveformdata_lsb_right));
				ofs.write((char*) (&waveformdata_msb_right), sizeof(waveformdata_msb_right));
			}
		}

		inline void read8Res1Channel(std::ifstream& ifs) {
			unsigned char waveformdata;
			data = new double[amountOfData];
			for (int i = 0; i < amountOfData; i++) {
				ifs.read((char*) (&waveformdata), sizeof(waveformdata));
				data[i] = (double) ((waveformdata));
			}
		}

		inline void read8Res2Channel(std::ifstream& ifs) {
			unsigned char waveformdata_right;
			unsigned char waveformdata_left;
			dataLeft = new double[amountOfData];
			dataRight = new double[amountOfData];
			for (int i = 0; i < amountOfData; i++) {
				ifs.read((char*) (&waveformdata_left), sizeof(waveformdata_left));
				ifs.read((char*) (&waveformdata_right), sizeof(waveformdata_right));
				dataLeft[i] = (double) ((waveformdata_right));
				dataRight[i] = (double) ((waveformdata_left));
			}
		}

		inline void read16Res1Channel(std::ifstream& ifs) {
			unsigned char waveformdata_lsb, waveformdata_msb;
			data = new double[amountOfData];
			for (int i = 0; i < amountOfData; i++) {
				ifs.read((char*) (&waveformdata_lsb), sizeof(waveformdata_lsb));
				ifs.read((char*) (&waveformdata_msb), sizeof(waveformdata_msb));
				data[i] = (double) ((convert2of8to1of16(waveformdata_lsb, waveformdata_msb)));
			}
		}

		inline void read16Res2Channel(std::ifstream& ifs) {
			unsigned char waveformdata_lsb_left, waveformdata_lsb_right, waveformdata_msb_left, waveformdata_msb_right;
			dataLeft = new double[amountOfData];
			dataRight = new double[amountOfData];
			for (int i = 0; i < amountOfData; i++) {
				ifs.read((char*) (&waveformdata_lsb_left), sizeof(waveformdata_lsb_left));
				ifs.read((char*) (&waveformdata_msb_left), sizeof(waveformdata_msb_left));
				ifs.read((char*) (&waveformdata_lsb_right), sizeof(waveformdata_lsb_right));
				ifs.read((char*) (&waveformdata_msb_right), sizeof(waveformdata_msb_right));
				dataLeft[i] = (double) ((convert2of8to1of16(waveformdata_lsb_left, waveformdata_msb_left)));
				dataRight[i] = (double) ((convert2of8to1of16(waveformdata_lsb_right, waveformdata_msb_right)));
			}
		}
};
#endif /* SRC_WAV_C_ */