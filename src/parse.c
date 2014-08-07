/*
 * owon-utils - a set of programs to use with OWON Oscilloscopes
 * Copyright (c) 2012  Lana Larsen <lana@stoatly.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "owon.h"
#include "parse.h"


//TODO: for tables, consider using functions to handle values out-of-bounds.
float _attenuation_table[] = {1.0e0, 1.0e1, 1.0e2, 1.0e3};

float _volt_table[] = {
            2.0e-3, 5.0e-3, // 1 mV
    1.0e-2, 2.0e-2, 5.0e-2, // 10 mV
    1.0e-1, 2.0e-1, 5.0e-1, // 100 mV
    1.0e+0, 2.0e+0, 5.0e+0, // 1 V
    1.0e+1, 2.0e+1, 5.0e+1, // 10 V
    1.0e+2, 2.0e+2, 5.0e+2, // 100 V
    1.0e+3, 2.0e+3, 5.0e+3, // 1 kV
    1.0e+4                  // 10 kV
};

// 1, 2, 5 step
float _time_table_10_20_50[] = {
    1.0e-9, 2.0e-9, 5.0e-9, // 1 ns
    1.0e-8, 2.0e-8, 5.0e-8, // 10 ns
    1.0e-7, 2.0e-7, 5.0e-7, // 100 ns
    1.0e-6, 2.0e-6, 5.0e-6, // 1 us
    1.0e-5, 2.0e-5, 5.0e-5, // 10 us
    1.0e-4, 2.0e-4, 5.0e-4, // 100 us
    1.0e-3, 2.0e-3, 5.0e-3, // 1 ms
    1.0e-2, 2.0e-2, 5.0e-2, // 10 ms
    1.0e-1, 2.0e-1, 5.0e-1, // 100 ms
    1.0e+0, 2.0e+0, 5.0e+0, // 1 s
    1.0e+1, 2.0e+1, 5.0e+1, // 10 s
    1.0e+2, 2.0e+2, 5.0e+2  // 100 s
};

// 1, 2.5, 5 step
float _time_table_10_25_50[] = {
    1.0e-9, 2.5e-9, 5.0e-9, // 1 ns
    1.0e-8, 2.5e-8, 5.0e-8, // 10 ns
    1.0e-7, 2.5e-7, 5.0e-7, // 100 ns
    1.0e-6, 2.5e-6, 5.0e-6, // 1 us
    1.0e-5, 2.5e-5, 5.0e-5, // 10 us
    1.0e-4, 2.5e-4, 5.0e-4, // 100 us
    1.0e-3, 2.5e-3, 5.0e-3, // 1 ms
    1.0e-2, 2.5e-2, 5.0e-2, // 10 ms
    1.0e-1, 2.5e-1, 5.0e-1, // 100 ms
    1.0e+0, 2.5e+0, 5.0e+0, // 1 s
    1.0e+1, 2.5e+1, 5.0e+1, // 10 s
    1.0e+2, 2.5e+2, 5.0e+2  // 100 s
};

float *_time_table_10_20_50__1 = &_time_table_10_20_50[0]; // start at 1ns
float *_time_table_10_20_50__2 = &_time_table_10_20_50[1]; // start at 2ns
float *_time_table_10_20_50__5 = &_time_table_10_20_50[2]; // start at 5ns
float *_time_table_10_25_50__5 = &_time_table_10_25_50[2]; // start at 5ns

float *get_attenuation_table(const char c) {
    // only one version across all models
    return _attenuation_table;
}

float *get_volt_table(const char c) {
    // only one version across all models
    return _volt_table;
}

// The table used is dependent on the type of model, which is indicated by 
// the 4th character in the file header. Example: SPBxyz, where `x` is the 
// character representing the model type.
// TODO: add support for SPCX01 (special type)
float *get_time_table(const char c) {
    switch(c) {
        case 'N': return _time_table_10_20_50__2;
        case 'M': return _time_table_10_20_50__1;
        case 'O': return _time_table_10_20_50__5;
        case 'P': return _time_table_10_20_50__5;
        case 'Q': return _time_table_10_20_50__5;
        case 'R': return _time_table_10_20_50__5;
        case 'S': return _time_table_10_20_50__5;
        case 'T': return _time_table_10_20_50__5;
        case 'U': return _time_table_10_20_50__5;
        case 'V': return _time_table_10_25_50__5;
        case 'W': return _time_table_10_20_50__5;
        case 'X': return _time_table_10_20_50__5;
        default: return NULL;
    }
};

//TODO: read Wave and FFT channels
//TODO: more helpful error handling
int owon_parse(struct owon_capture *capture, FILE *fp) {
    memset(capture, 0, sizeof(*capture));

    struct owon_header file_header;
    memset(&file_header, 0, sizeof(file_header));

    fread(&file_header.header, sizeof(char), sizeof(file_header.header), fp);
    strncpy((char *)&capture->header, file_header.header, sizeof(file_header.header));

    // TODO: not all models start with `SPB`!
    // Ensure file is the right format.
    if (0 != strncmp("SPB", (const char *)&file_header.header, 3)) {
        return OWON_ERROR_HEADER;
    }
   
    // The 4th character (index 3) of the header string indicate which tables
    // must be used.
    char model = file_header.header[3];
    float *attenuation_table = get_attenuation_table(model);
    float *volt_table = get_volt_table(model);
    float *time_table = get_time_table(model);
    if (attenuation_table == NULL || volt_table == NULL || 
            time_table == NULL) {
        return OWON_ERROR_UNSUPPORTED;
    }

    fread(&file_header.length, sizeof(int), 1, fp);

    // Custom models are indicated a negative length
    if (file_header.length < 0) {
        return OWON_ERROR_UNSUPPORTED;
    }

    // TODO: maybe try to calculate how many channels there are based on the 
    // length and number of samples? The number of samples may not be 
    // consistant between channels.
    capture->channels = calloc(OWON_MAX_CHANNELS, 
            sizeof(struct owon_channel));
    if (NULL == capture->channels) {
        return OWON_ERROR_MEMORY;
    }
    capture->channel_count = 0;

    while (ftell(fp) < file_header.length) {
        struct owon_channel_header chan_header;
        fread(&chan_header.name, sizeof(char), sizeof(chan_header.name), fp);
        fread(&chan_header.length, sizeof(int), 1, fp);
        fread(&chan_header.sample_count, sizeof(int), 1, fp);
        fread(&chan_header.sample_screen, sizeof(int), 1, fp);
        fread(&chan_header.slow_scan_pos, sizeof(int), 1, fp);
        fread(&chan_header.time_div, sizeof(int), 1, fp);
        fread(&chan_header.zero_point, sizeof(int), 1, fp);
        fread(&chan_header.volts_div, sizeof(int), 1, fp);
        fread(&chan_header.attenuation, sizeof(int), 1, fp);
        fread(&chan_header.time_mul, sizeof(float), 1, fp);
        fread(&chan_header.frequency, sizeof(float), 1, fp);
        fread(&chan_header.period, sizeof(float), 1, fp);
        fread(&chan_header.volts_mul, sizeof(float), 1, fp);
        
        struct owon_channel *channel;
        channel = &capture->channels[capture->channel_count];
        strncpy((char *)&channel->name, chan_header.name, 
                sizeof(chan_header.name));
        channel->attenuation = attenuation_table[chan_header.attenuation];
        channel->volts_mul = chan_header.volts_mul;
        channel->volts_div = volt_table[chan_header.volts_div];
        channel->time_mul = chan_header.time_mul;
        channel->time_div = time_table[chan_header.time_div];
        channel->frequency = chan_header.frequency;
        channel->period = channel->period;
        channel->sample_count = chan_header.sample_count;
        channel->samples = malloc(chan_header.sample_count * sizeof(short));
        if (NULL == channel->samples) {
            return OWON_ERROR_MEMORY;
        }
        fread(channel->samples, sizeof(short), channel->sample_count, fp);

        if (feof(fp) || ferror(fp)) {
            return OWON_ERROR_READ;
        }

        capture->channel_count++;
    }

    return OWON_SUCCESS;
}

void owon_free_capture(struct owon_capture *capture) {
    while (capture->channel_count--) {
        free(capture->channels[capture->channel_count].samples);
    }
    free(capture->channels);
}

int owon_write_delim(struct owon_capture const *capture, char *delim,
        char *line_end, int header, FILE *fp) {
    if (capture->channel_count < 1) {
        return OWON_ERROR;
    }
    if (header) {
        fprintf(fp, "Time (us)%s", delim);
    }
    int chan_idx;
    int max_samples = 0;
    for (chan_idx = 0; chan_idx < capture->channel_count; chan_idx++) {
        struct owon_channel *channel = &capture->channels[chan_idx];
        if (channel->sample_count > max_samples) {
            max_samples = channel->sample_count;
        }
        if (header) {
            char *s;
            if (chan_idx < capture->channel_count - 1) {
                s = delim;
            } else {
                s = line_end;
            }
            fprintf(fp, "%s (mV)%s", channel->name, s);
        }
    }
    int sample_idx;
    for (sample_idx = 0; sample_idx < max_samples; sample_idx++) {
        fprintf(fp, "%f%s", sample_idx * capture->channels[0].time_mul, delim);
        for (chan_idx = 0; chan_idx < capture->channel_count; chan_idx++) {
            struct owon_channel *channel = &capture->channels[chan_idx];
            char *s;
            if (chan_idx < capture->channel_count - 1) {
                s = delim;
            } else {
                s = line_end;
            }
            if (sample_idx < channel->sample_count) {
                fprintf(fp, "%f%s", channel->samples[sample_idx] * 
                        channel->volts_mul * channel->attenuation, s);
            } else {
                fprintf(fp, " %s", s);
            }
        }
    }
    return OWON_SUCCESS;
}

