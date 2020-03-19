#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>

//void HtoRGB( float &fR, float &fG, float &fB, const float &fH );
void HtoRGB( uint8_t &r, uint8_t &g, uint8_t &b, const float &fH );
void RGBtoHSV( const float &fR, const float &fG, const float &fB, float &fH, float &fS, float &fV );
void HSVtoRGB( float &fR, float &fG, float &fB, const float &fH, const float &fS, const float &fV );

#endif // COLOR_HPP