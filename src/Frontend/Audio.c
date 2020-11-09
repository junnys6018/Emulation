#include "Audio.h"
#include "Frontend/Controller.h"
#include "Backend/2A03.h"

#include <string.h>
#include <stdio.h>

#define BUF_SIZE 2048
#define BUF_COUNT 3

typedef struct
{
	// For DC cutoff filter
	float last_sample;
	float last_filter;
} Userdata;

typedef struct
{
	float* volatile buffers;

	// Number of free buffers that can we writen to
	SDL_sem* volatile sem_free_buffers;

	// Current buffer we are reading from
	volatile int read_buffer;

	// Current buffer we are writing to
	int write_buffer;

	// Index into the write buffer indicating the next free sample to write to
	int write_pos;

	SDL_AudioDeviceID audio_handle;
} AudioContext;

static AudioContext ac;
static Userdata userdata;

float absf(float f)
{
	return f < 0.0f ? -f : f;
}

void audio_callback(void* userdata, Uint8* stream, int len)
{
	// If not all the buffers are free (ie some buffers have been written to)
	if (SDL_SemValue(ac.sem_free_buffers) < BUF_COUNT - 1)
	{
		float* my_stream = (float*)stream;
		Userdata* data = (Userdata*)userdata;

		for (int i = 0; i < len / sizeof(float); i++)
		{
			// Filter given samples with a DC cutoff filter. 
			// This is because the NES outputs audio in the [0,1] range, but our audio drivers expect
			// audio samples in the [-1,1] range.
			float sample = ac.buffers[BUF_SIZE * ac.read_buffer + i];

			float filtered = sample - data->last_sample + 0.995f * data->last_filter;
			data->last_sample = sample;
			data->last_filter = filtered;
			my_stream[i] = filtered;

			if (!(absf(my_stream[i]) <= 1.0f))
			{
				my_stream[i] = 0.0f;
				printf("[ERROR] sound\n");
			}
		}

		// Read from the next buffer in the next callback
		ac.read_buffer = (ac.read_buffer + 1) % BUF_COUNT;

		// Increment the semaphore indicating another buffer is free for writing to
		SDL_SemPost(ac.sem_free_buffers);
	}
	else
	{
		memset(stream, 0, len);
	}
}

void InitSDLAudio()
{
	ac.buffers = malloc(BUF_SIZE * BUF_COUNT * sizeof(float));
	ac.sem_free_buffers = SDL_CreateSemaphore(BUF_COUNT - 1);
	if (!ac.sem_free_buffers)
	{
		printf("[Error] Failed to create semaphore: %s\n", SDL_GetError());
	}
	ac.read_buffer = 0;
	ac.write_buffer = 0;
	ac.write_pos = 0;

	userdata.last_filter = 0.0f;
	userdata.last_sample = 0.0f;

	SDL_AudioSpec spec, have;
	memset(&spec, 0, sizeof(spec));

	spec.freq = SAMPLE_RATE;
	spec.format = AUDIO_F32;
	spec.channels = 1;
	spec.samples = BUF_SIZE;
	spec.callback = audio_callback;
	spec.userdata = &userdata;

	ac.audio_handle = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);

	// Play audio
	SDL_PauseAudioDevice(ac.audio_handle, 0);
}

void ShutdownSDLAudio()
{
	free(ac.buffers);
	SDL_DestroySemaphore(ac.sem_free_buffers);
	SDL_PauseAudioDevice(ac.audio_handle, 1);
	SDL_CloseAudioDevice(ac.audio_handle);
}

void WriteSamples(const float* in, int count)
{
	// While there are still samples to be written
	while (count)
	{
		// Write as many samples we can into the current audio buffer
		int num_samples_write = BUF_SIZE - ac.write_pos;
		if (num_samples_write > count)
		{
			num_samples_write = count;
		}

		memcpy(ac.buffers + (long)BUF_SIZE * ac.write_buffer + ac.write_pos, in, num_samples_write * sizeof(float));
		in += num_samples_write;
		ac.write_pos += num_samples_write;
		count -= num_samples_write;

		// If we filled the current write buffer
		if (ac.write_pos == BUF_SIZE)
		{
			// Reset the write pointer
			ac.write_pos = 0;

			// Start writing to the next buffer
			ac.write_buffer = (ac.write_buffer + 1) % BUF_COUNT;

			// Block until there a buffer is free for writing
			SDL_SemWait(ac.sem_free_buffers);
		}
	}
}