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

#ifndef __OWON__PARSE_H__
#define __OWON__PARSE_H__

// TODO: consider using stdint.h for header data types?

#define OWON_MAX_CHANNELS 6

struct owon_header {
    char header[6];
    int length;
};

// Raw channel header as it comes from the device.
struct owon_channel_header {
    char name[3];       // Channel name (CH1, CH2, CHA, CHB, CHC, CHD.
                        // CHA-D are from wave save, Cf1 is FFT. 
                        // Not null-terminated!
    int length;         // Length from after channel name to end of data 
                        // section (sample length + 48). The total size of 
                        // the channel header is 51 bytes.
    int sample_count;   // Number of samples.
    int sample_screen;  // Number of samples shown on screen. This value 
                        // differ from `sample_count` when in 
                        // "slow scanner mode" (see below).
    int slow_scan_pos;  // When slow scanning (timebase >= 100 ms), the 
                        // relative sample point position on screen, 
                        // otherwise, this value is normally 0.
    int time_div;       // Horizontal scale index (sec/div); use table.
    int zero_point;     // +/- distance from zeropoint. 1 = 0.04 divisions.
    int volts_div;      // Vertical scale index (volts/div); use table.
    int attenuation;    // Probe attenuation index; use table.
    float time_mul;     // The time, in us, between each sample. Multiply 
                        // by the sample index to get the relative time a 
                        // sample was captured.
    float frequency;    // Frequency in Hz.
    float period;       // Period in us.
    float volts_mul;    // The voltage multipler, in mV, not taking in 
                        // account attenuation, for each sample point.
                        // Multiple this value by the attenuation and the 
                        // sample point value to get the actual value in mV.
};

struct owon_channel {
    char name[4]; // 3 characters plus a null terminator
    float attenuation;
    float volts_mul;
    float volts_div;
    float time_mul;
    float time_div;
    float frequency;
    float period;
    int sample_count;
    short *samples;
};

struct owon_capture {
    char header[7]; // 6 characters plus a null terminator
    int channel_count;
    struct owon_channel *channels;
};

float *get_attenuation_table(const char c);
float *get_volt_table(const char c);
float *get_time_table(const char c);
int owon_parse(struct owon_capture *capture, FILE *fp);
void owon_free_capture(struct owon_capture *capture);
int owon_write_delim(struct owon_capture const *capture, char *delim, 
        char *line_end, int header, FILE *fp);

#endif // __OWON__PARSE_H__
