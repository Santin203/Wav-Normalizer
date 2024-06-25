/* Example code that parses the header of a .wav file */
/* By Santiago Jim�nez */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>
#include "cfunctions_headers.h"

#define MAX_ITEM_SIZE 200
#define BLOCK_SIZE_MS 100

// Function to write the floating-point audio data for left and right channels
void writeFloatData(FILE* file, float* left_channel, float* right_channel, int num_samples) {
	for (int i = 0; i < num_samples; i++) {
		fwrite(&left_channel[i], sizeof(float), 1, file);
		fwrite(&right_channel[i], sizeof(float), 1, file);
	}
}

// Read the floating-point audio data for left and right channels
void readFloatData(FILE* file, float* left_channel, float* right_channel, int num_samples) {
	float sample;
	for (int i = 0; i < num_samples; i++) {
		fread(&sample, sizeof(float), 1, file);
		left_channel[i] = sample;
		fread(&sample, sizeof(float), 1, file);
		right_channel[i] = sample;
	}

	// Add a -100.0 sample to the end of each channel to flag the end of the samples
	left_channel[num_samples-1] = -100.0;
	right_channel[num_samples-1] = -100.0;
}

int main(void)
{
	FILE* infile;
	FILE* outfile;
	char item[MAX_ITEM_SIZE];
	int	 chunk_size;
	int  subchunk_size;
	int  processed_bytes = 0;
	short shortint;
	int   longint;

	int size;
	int block_size;
	int num_blocks;

	int count;
	float sample;
	float* psamples;


	float* left_channel_data;
	float* right_channel_data;

	float* left_channel_average_volumes;
	float* right_channel_average_volumes;

	LARGE_INTEGER start_time, end_time, frequency;

	// Get the frequency of the performance counter
	QueryPerformanceFrequency(&frequency);


	infile = fopen("Kid A.wav", "rb");
	if (infile == NULL)
	{
		printf("Could not open file! \n");
		return (-1);
	}

	outfile = fopen("output.wav", "wb");
	if (outfile == NULL) {
		printf("Could not create output file! \n");
		return (-1);
	}

	/* Chunk ID: 4-byte string */
	fread(item, 4, 1, infile);
	item[4] = 0;
	printf("Chunk ID: %s\n", item);
	fwrite(item, 4, 1, outfile);

	/* Chunk size: 32-bit int */
	fread(&chunk_size, 4, 1, infile);
	printf("Chunk size: %d\n", chunk_size);
	fwrite(&chunk_size, 4, 1, outfile);

	/* Chunk format: 4-byte string */
	fread(item, 4, 1, infile);
	item[4] = 0;
	printf("Chunk format: %s\n", item);
	fwrite(item, 4, 1, outfile);
	processed_bytes = 4;

	/* Now we will process sub-chunks */
	while (processed_bytes < chunk_size)
	{
		/* Subchunk ID: 4-byte string */
		fread(item, 4, 1, infile);
		item[4] = 0;
		printf("Subchunk ID: %s\n", item);
		fwrite(item, 4, 1, outfile);
		processed_bytes += 4;

		/* Subchunk size: 32-bit int */
		fread(&subchunk_size, 4, 1, infile);
		printf("  Subchunk size: %d\n", subchunk_size);
		fwrite(&subchunk_size, 4, 1, outfile);
		processed_bytes += 4;

		/* Process subchunk data */

		if (strcmp(item, "fmt ") == 0)
		{
			/* Audio format: 2-byte integer */
			fread(&shortint, 2, 1, infile);
			printf("  Audio format: %d\n", shortint);
			fwrite(&shortint, 2, 1, outfile);

			/* Number of channels: 2-byte integer */
			fread(&shortint, 2, 1, infile);
			printf("  Number of channels: %d\n", shortint);
			fwrite(&shortint, 2, 1, outfile);

			/* Sample rate: 4-byte integer */
			fread(&longint, 4, 1, infile);
			printf("  Sample rate: %d\n", longint);
			fwrite(&longint, 4, 1, outfile);

			/* Byte rate: 4-byte integer */
			fread(&longint, 4, 1, infile);
			printf("  Byte rate: %d\n", longint);
			fwrite(&longint, 4, 1, outfile);

			/* Block alignment: 2-byte integer */
			fread(&shortint, 2, 1, infile);
			printf("  Block alignment: %d\n", shortint);
			fwrite(&shortint, 2, 1, outfile);

			/* Bits per sample: 2-byte integer */
			fread(&shortint, 2, 1, infile);
			printf("  Bits per sample: %d\n", shortint);
			fwrite(&shortint, 2, 1, outfile);
		}
		else if (strcmp(item, "data") == 0)
		{
			printf("  Reading data samples\n");

			// Calculate the block size for 100ms
			block_size = 44100 * BLOCK_SIZE_MS / 1000; 

			// Calculate the number of samples
			size = subchunk_size / sizeof(float) / 2; 

			// Allocate memory for left and right channel data
			left_channel_data = (float*)calloc(subchunk_size / 2, sizeof(float));
			right_channel_data = (float*)calloc(subchunk_size / 2, sizeof(float));

			// Read the floating-point audio data for left and right channels
			readFloatData(infile, left_channel_data, right_channel_data, size);

			// Calculate the number of blocks
			num_blocks = (size / block_size) + 1;

			// Allocate memory for average volume for each block
			left_channel_average_volumes = (float*)calloc(num_blocks, sizeof(float));
			right_channel_average_volumes = (float*)calloc(num_blocks, sizeof(float));

			// Start the timer
			QueryPerformanceCounter(&start_time);

			// Calculate average volume for each block
			for (int i = 0; i < num_blocks; i++) {
				// Get start sample(index) for current block
				int start_sample = i * block_size;

				// Calculate average volume for current block
				left_channel_average_volumes[i] = calculateAverageVolume(&left_channel_data[start_sample], block_size);
				right_channel_average_volumes[i] = calculateAverageVolume(&right_channel_data[start_sample], block_size);
			}

			// Stop the timer
			QueryPerformanceCounter(&end_time);

			// Calculate the elapsed time
			double elapsed_time = (end_time.QuadPart - start_time.QuadPart) * 1000 / frequency.QuadPart;
			printf("Time taken for calculateAverageVolume(): %lf ms\n", elapsed_time);

			// Find maximum average volume for each channel
			float max_left_channel_volume = 0.0;
			float max_right_channel_volume = 0.0;

			// Start the timer
			QueryPerformanceCounter(&start_time);

			// Store the maximum average volume for each channel
			max_left_channel_volume = findReferenceVolume(left_channel_average_volumes, num_blocks);
			max_right_channel_volume = findReferenceVolume(right_channel_average_volumes, num_blocks);

			// Stop the timer
			QueryPerformanceCounter(&end_time);

			// Calculate the elapsed time
			elapsed_time = (end_time.QuadPart - start_time.QuadPart) * 1000000 / frequency.QuadPart;
			printf("Time taken for findReferenceVolume(): %lf us\n", elapsed_time);

			//printf("Max left channel volume: %f\n", max_left_channel_volume);
			//printf("Max right channel volume: %f\n", max_right_channel_volume);

			// Allocate memory for amplification factors for each block
			float* left_channel_amplification_factors = (float*)malloc(num_blocks * sizeof(float));
			float* right_channel_amplification_factors = (float*)malloc(num_blocks * sizeof(float));

			// Start the timer
			QueryPerformanceCounter(&start_time);

			// Calculate amplification factors for each block
			computeAmplificationFactors(left_channel_average_volumes, max_left_channel_volume, left_channel_amplification_factors, num_blocks);
			computeAmplificationFactors(right_channel_average_volumes, max_right_channel_volume, right_channel_amplification_factors, num_blocks);

			// Stop the timer
			QueryPerformanceCounter(&end_time);

			// Calculate the elapsed time
			elapsed_time = (end_time.QuadPart - start_time.QuadPart) * 1000000 / frequency.QuadPart;
			printf("Time taken for computeAmplificationFactors(): %lf us\n", elapsed_time);

			// Start the timer
			QueryPerformanceCounter(&start_time);

			// Apply amplification factors to each block
			for (int i = 0; i < num_blocks; i++) {
				// Get start sample(index) for current block
				int start_sample = i * block_size;

				// Apply amplification factor for current block
				applyAmplificationFactor(&left_channel_data[start_sample], left_channel_amplification_factors[i], block_size);
				applyAmplificationFactor(&right_channel_data[start_sample], right_channel_amplification_factors[i], block_size);
			}

			// Stop the timer
			QueryPerformanceCounter(&end_time);

			// Calculate the elapsed time
			elapsed_time = (end_time.QuadPart - start_time.QuadPart) * 1000 / frequency.QuadPart;
			printf("Time taken for applyAmplificationFactor(): %lf ms\n", elapsed_time);

			// Write the modified floating-point audio data for left and right channels
			writeFloatData(outfile, left_channel_data, right_channel_data, size);

			// Free memory
			free(left_channel_data);
			free(right_channel_data);
			free(left_channel_average_volumes);
			free(right_channel_average_volumes);
			free(left_channel_amplification_factors);
			free(right_channel_amplification_factors);

		}
		else
		{
			/* Unknown subchunk */
			printf("  Ignoring subchunk\n");
			fread(item, subchunk_size, 1, infile);
			fwrite(item, subchunk_size, 1, outfile);
		}

		processed_bytes += subchunk_size;
	}
	// Close files
	fclose(infile);
	fclose(outfile);
	printf("All done\n");
}