/*
Copyright (C) 2017-2025 J. Williams

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* Higher-level drawing functions handling display of the user interface. */
/* The UI scale shouldn't be handled here, it should instead be handled in the lower level functions of state->ds.sds.h*/

#include "gui.h"
#include "gui_constants.h"
#include "juicer.h" //contains easing functions used in animations
#include "drawing.h"
#include "data_ops.h"

float getAxisTickSpacing(float zoomScale){
  //SDL_Log("range: %f\n",(double)range);
  if(zoomScale > 12.0f){
    return 1.0f;
  }else if(zoomScale > 6.0f){
    return 2.0f;
  }else if(zoomScale > 3.5f){
    return 4.0f;
  }else if(zoomScale > 1.5f){
    return 10.0f;
  }else if(zoomScale > 0.9f){
    return 20.0f;
  }else{
    return 40.0f;
  }
}

SDL_FColor getHalfLifeCol(const double halflifeSeconds){
  //SDL_Log("half-life: %0.6f\n",halflifeSeconds);
  SDL_FColor col;
  col.r = 1.0f;
  col.g = 1.0f;
  col.b = 1.0f;
  col.a = 1.0f;
  if(halflifeSeconds > 1.0E15){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(halflifeSeconds > 1.0E10){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(halflifeSeconds > 1.0E8){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 5.0E7){
    col.r = 0.1f;
    col.g = 0.15f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 1.0E7){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 5.0E6){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.55f;
  }else if(halflifeSeconds > 1.0E6){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.6f;
  }else if(halflifeSeconds > 5.0E5){
    col.r = 0.1f;
    col.g = 0.25f;
    col.b = 0.65f;
  }else if(halflifeSeconds > 1.0E5){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 5.0E4){
    col.r = 0.15f;
    col.g = 0.35f;
    col.b = 0.75f;
  }else if(halflifeSeconds > 1.0E4){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 5.0E3){
    col.r = 0.25f;
    col.g = 0.45f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 1.0E3){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 5.0E2){
    col.r = 0.3f;
    col.g = 0.6f;
    col.b = 0.75f;
  }else if(halflifeSeconds > 1.0E2){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 5.0E1){
    col.r = 0.3f;
    col.g = 0.8f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E1){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 5.0){
    col.r = 0.4f;
    col.g = 0.95f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 5.0E-1){
    col.r = 0.6f;
    col.g = 0.95f;
    col.b = 0.6f;
  }else if(halflifeSeconds > 1.0E-1){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 5.0E-2){
    col.r = 0.8f;
    col.g = 0.9f;
    col.b = 0.45f;
  }else if(halflifeSeconds > 1.0E-2){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(halflifeSeconds > 5.0E-3){
    col.r = 0.95f;
    col.g = 0.95f;
    col.b = 0.35f;
  }else if(halflifeSeconds > 1.0E-3){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(halflifeSeconds > 5.0E-4){
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.35f;
  }else if(halflifeSeconds > 1.0E-4){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(halflifeSeconds > 5.0E-5){
    col.r = 1.0f;
    col.g = 0.75f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 1.0E-5){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.6f;
  }else if(halflifeSeconds > 5.0E-6){
    col.r = 1.0f;
    col.g = 0.65f;
    col.b = 0.65f;
  }else if(halflifeSeconds > 1.0E-6){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 5.0E-7){
    col.r = 1.0f;
    col.g = 0.65f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E-7){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E-8){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 1.0E-9){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 1.0E-12){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else{
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  return col;
}

SDL_FColor getDecayModeCol(const uint8_t dcyMode){
  SDL_FColor col;
  col.r = 1.0f;
  col.g = 1.0f;
  col.b = 1.0f;
  col.a = 1.0f;
  switch(dcyMode){
    case DECAYMODE_BETAPLUS:
      col.r = 0.8f;
      col.g = 0.6f;
      col.b = 1.0f;
      break;
    case DECAYMODE_2BETAPLUS:
      col.r = 0.4f;
      col.g = 0.3f;
      col.b = 0.5f;
      break;
    case DECAYMODE_EC:
      col.r = 0.6f;
      col.g = 0.8f;
      col.b = 1.0f;
      break;
    case DECAYMODE_2EC:
      col.r = 0.3f;
      col.g = 0.4f;
      col.b = 0.5f;
      break;
    case DECAYMODE_ECANDBETAPLUS:
      col.r = 0.8f;
      col.g = 0.8f;
      col.b = 1.0f;
      break;
    case DECAYMODE_BETAMINUS:
      col.r = 1.0f;
      col.g = 0.6f;
      col.b = 1.0f;
      break;
    case DECAYMODE_2BETAMINUS:
      col.r = 0.5f;
      col.g = 0.3f;
      col.b = 0.5f;
      break;
    case DECAYMODE_ALPHA:
    case DECAYMODE_BETAMINUS_ALPHA:
    case DECAYMODE_BETAPLUS_ALPHA:
    case DECAYMODE_EC_ALPHA:
      col.r = 1.0f;
      col.g = 1.0f;
      col.b = 0.6f;
      break;
    case DECAYMODE_PROTON:
    case DECAYMODE_TWOPROTON:
    case DECAYMODE_BETAPLUS_PROTON:
    case DECAYMODE_BETAPLUS_TWOPROTON:
    case DECAYMODE_BETAPLUS_THREEPROTON:
    case DECAYMODE_EC_PROTON:
    case DECAYMODE_EC_TWOPROTON:
    case DECAYMODE_EC_THREEPROTON:
    case DECAYMODE_BETAMINUS_PROTON:
      col.r = 1.0f;
      col.g = 0.6f;
      col.b = 0.6f;
      break;
    case DECAYMODE_NEUTRON:
    case DECAYMODE_TWONEUTRON:
    case DECAYMODE_BETAMINUS_NEUTRON:
    case DECAYMODE_BETAMINUS_TWONEUTRON:
      col.r = 0.6f;
      col.g = 0.6f;
      col.b = 1.0f;
      break;
    case DECAYMODE_IT:
      col.r = 1.0f;
      col.g = 0.5f;
      col.b = 0.5f;
      break;
    case DECAYMODE_SPONTANEOUSFISSION:
    case DECAYMODE_BETAMINUS_SPONTANEOUSFISSION:
      col.r = 0.6f;
      col.g = 1.0f;
      col.b = 0.6f;
      break;
    case (DECAYMODE_ENUM_LENGTH+1):
      //stable
      col.r = 0.0f;
      col.g = 0.0f;
      col.b = 0.0f;
      break;
    case DECAYMODE_ENUM_LENGTH:
    default:
      //no decay mode found
      col.r = 0.7f;
      col.g = 0.7f;
      col.b = 0.7f;
      break;
  }
  return col;
}

SDL_Color getDecayModeTextCol(const uint8_t dcyMode){
  const SDL_FColor bgCol = getDecayModeCol(dcyMode);
  if((bgCol.r + bgCol.g + bgCol.b) < 2.0f){
    return whiteCol8Bit;
  }
  return blackCol8Bit;
}

SDL_FColor get2PlusCol(const double e2PlusKeV, const double halflifeSeconds){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(e2PlusKeV >= 8000.0){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(e2PlusKeV >= 7000.0){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(e2PlusKeV >= 6000.0){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(e2PlusKeV >= 5000.0){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 4000.0){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(e2PlusKeV >= 3000.0){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(e2PlusKeV >= 2500.0){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 2000.0){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 1500.0){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 1200.0){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(e2PlusKeV >= 900.0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(e2PlusKeV >= 700.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(e2PlusKeV >= 500.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(e2PlusKeV >= 400.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(e2PlusKeV >= 300.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(e2PlusKeV >= 200.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 100.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(e2PlusKeV >= 50.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(e2PlusKeV > 0.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  //slightly darken stable nuclides
  if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }
  return col;
}

SDL_FColor getR42Col(const double r42, const double halflifeSeconds){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(r42 >= 5.0){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(r42 >= 3.8){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(r42 >= 3.5){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(r42 >= 3.4){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(r42 >= 3.3){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(r42 >= 3.2){ //box color inversion point
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(r42 >= 3.1){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(r42 >= 2.8){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(r42 >= 2.5){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(r42 >= 2.2){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(r42 >= 2.1){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(r42 >= 2.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(r42 >= 1.9){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(r42 >= 1.8){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(r42 >= 1.4){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(r42 >= 1.3){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(r42 >= 1.2){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(r42 >= 1.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(r42 > 0.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  //slightly darken stable nuclides
  if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }
  return col;
}

SDL_FColor getBeta2Col(const double beta2, const double halflifeSeconds){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(beta2 >= 0.8){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(beta2 >= 0.7){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(beta2 >= 0.6){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(beta2 >= 0.5){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(beta2 >= 0.4){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(beta2 >= 0.35){ //box color inversion point
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(beta2 >= 0.3){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(beta2 >= 0.28){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(beta2 >= 0.26){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(beta2 >= 0.24){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(beta2 >= 0.22){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(beta2 >= 0.2){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(beta2 >= 0.15){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(beta2 >= 0.1){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(beta2 >= 0.08){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(beta2 >= 0.06){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(beta2 >= 0.04){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(beta2 >= 0.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  //slightly darken stable nuclides
  if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }
  return col;
}

SDL_FColor getSpinCol(const double spin){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(spin <= 0.0){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(spin <= 0.5){
    col.r = 0.05f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(spin <= 1.0){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.6f;
  }else if(spin <= 1.5){
    col.r = 0.25f;
    col.g = 0.4f;
    col.b = 0.7f;
  }else if(spin <= 2.0){ //box color inversion point
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(spin <= 2.5){
    col.r = 0.3f;
    col.g = 0.6f;
    col.b = 0.75f;
  }else if(spin <= 3.0){
    col.r = 0.35f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(spin <= 3.5){
    col.r = 0.4f;
    col.g = 0.8f;
    col.b = 0.7f;
  }else if(spin <= 4.0){
    col.r = 0.5f;
    col.g = 0.9f;
    col.b = 0.6f;
  }else if(spin <= 4.5){
    col.r = 0.6f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(spin <= 5.0){
    col.r = 0.7f;
    col.g = 0.95f;
    col.b = 0.4f;
  }else if(spin <= 5.5){
    col.r = 0.8f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(spin <= 6.0){
    col.r = 0.9f;
    col.g = 0.95f;
    col.b = 0.3f;
  }else if(spin <= 6.5){
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.35f;
  }else if(spin <= 7.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(spin <= 8.0){
    col.r = 1.0f;
    col.g = 0.75f;
    col.b = 0.45f;
  }else if(spin <= 9.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(spin <= 11.0){
    col.r = 1.0f;
    col.g = 0.65f;
    col.b = 0.55f;
  }else if(spin <= 13.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(spin <= 15.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(spin <= 17.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(spin < 255.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else{
    //unknown spin
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.9f;
  }
  return col;
}

SDL_FColor getParCol(const int8_t par){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(par <= -1.0){
    col.r = 0.2f;
    col.g = 0.2f;
    col.b = 0.2f;
  }else if(par >= 1.0){
    col.r = 0.8f;
    col.g = 0.8f;
    col.b = 0.8f;
  }else{
    //unknown parity
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.9f;
  }
  return col;
}

SDL_FColor getBEACol(const double beA){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(beA >= 8750.0){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(beA >= 8700.0){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.3f;
  }else if(beA >= 8650.0){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(beA >= 8600.0){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(beA >= 8550.0){
    col.r = 0.1f;
    col.g = 0.25f;
    col.b = 0.6f;
  }else if(beA >= 8500.0){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(beA >= 8450.0){
    col.r = 0.15f;
    col.g = 0.3f;
    col.b = 0.75f;
  }else if(beA >= 8400.0){
    col.r = 0.2f;
    col.g = 0.3f;
    col.b = 0.8f;
  }else if(beA >= 8350.0){
    col.r = 0.2f;
    col.g = 0.35f;
    col.b = 0.8f;
  }else if(beA >= 8300.0){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(beA >= 8250.0){
    col.r = 0.2f;
    col.g = 0.45f;
    col.b = 0.8f;
  }else if(beA >= 8200.0){
    col.r = 0.2f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(beA >= 8150.0){
    col.r = 0.25f;
    col.g = 0.55f;
    col.b = 0.8f;
  }else if(beA >= 8100.0){
    col.r = 0.3f;
    col.g = 0.6f;
    col.b = 0.8f;
  }else if(beA >= 8050.0){
    col.r = 0.3f;
    col.g = 0.6f;
    col.b = 0.75f;
  }else if(beA >= 8000.0){ //box color inversion point
    col.r = 0.3f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(beA >= 7950.0){
    col.r = 0.3f;
    col.g = 0.65f;
    col.b = 0.7f;
  }else if(beA >= 7900.0){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(beA >= 7850.0){
    col.r = 0.3f;
    col.g = 0.75f;
    col.b = 0.7f;
  }else if(beA >= 7800.0){
    col.r = 0.3f;
    col.g = 0.8f;
    col.b = 0.7f;
  }else if(beA >= 7750.0){
    col.r = 0.3f;
    col.g = 0.85f;
    col.b = 0.7f;
  }else if(beA >= 7700.0){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(beA >= 7650.0){
    col.r = 0.35f;
    col.g = 0.95f;
    col.b = 0.7f;
  }else if(beA >= 7600.0){
    col.r = 0.4f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(beA >= 7550.0){
    col.r = 0.4f;
    col.g = 0.95f;
    col.b = 0.7f;
  }else if(beA >= 7500.0){
    col.r = 0.5f;
    col.g = 0.95f;
    col.b = 0.7f;
  }else if(beA >= 7450.0){
    col.r = 0.5f;
    col.g = 0.9f;
    col.b = 0.6f;
  }else if(beA >= 7400.0){
    col.r = 0.6f;
    col.g = 0.9f;
    col.b = 0.6f;
  }else if(beA >= 7300.0){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(beA >= 7200.0){
    col.r = 0.8f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(beA >= 7100.0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(beA >= 6800.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(beA >= 6500.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(beA >= 6200.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(beA >= 5900.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(beA >= 5600.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(beA >= 5300.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(beA >= 4500.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(beA > 0.0){
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.9f;
  }
  return col;
}

SDL_FColor getSnpCol(const double snpKeV){
  SDL_FColor col;
  col.a = 1.0f;
  if(snpKeV == 0.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 1.0f;
  }else if(snpKeV >= 25000.0){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(snpKeV >= 20000.0){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(snpKeV >= 15000.0){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(snpKeV >= 10000.0){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(snpKeV >= 8000.0){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(snpKeV >= 6000.0){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(snpKeV >= 5000.0){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(snpKeV >= 4000.0){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(snpKeV >= 3000.0){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(snpKeV >= 2000.0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(snpKeV >= 1000.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(snpKeV >= 500.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(snpKeV >= 300.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(snpKeV >= 200.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(snpKeV >= 100.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(snpKeV >= 50.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(snpKeV >= 0.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(snpKeV >= -100.0){
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.9f;
  }else{
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 1.0f;
  }
  //slightly darken stable nuclides
  /*if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }*/
  return col;
}

SDL_FColor getQaCol(const double qaKeV){
  SDL_FColor col;
  col.a = 1.0f;
  if(qaKeV == 0.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 1.0f;
  }else if(qaKeV >= 10000.0){
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.9f;
  }else if(qaKeV >= 9000.0){
    col.r = 1.0f;
    col.g = 0.85f;
    col.b = 0.9f;
  }else if(qaKeV >= 8000.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(qaKeV >= 7000.0){
    col.r = 1.0f;
    col.g = 0.75f;
    col.b = 0.85f;
  }else if(qaKeV >= 6000.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(qaKeV >= 5000.0){
    col.r = 1.0f;
    col.g = 0.65f;
    col.b = 0.8f;
  }else if(qaKeV >= 4000.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.8f;
  }else if(qaKeV >= 3000.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(qaKeV >= 2000.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(qaKeV >= 1000.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(qaKeV >= 500.0){
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.3f;
  }else if(qaKeV >= 250.0){
    col.r = 1.0f;
    col.g = 0.95f;
    col.b = 0.2f;
  }else if(qaKeV >= 0.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.1f;
  }else if(qaKeV >= -250.0){
    col.r = 0.95f;
    col.g = 0.95f;
    col.b = 0.2f;
  }else if(qaKeV >= -500.0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.3f;
  }else if(qaKeV >= -1000.0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(qaKeV >= -1500.0){
    col.r = 0.8f;
    col.g = 0.9f;
    col.b = 0.45f;
  }else if(qaKeV >= -2000.0){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(qaKeV >= -2500.0){
    col.r = 0.65f;
    col.g = 0.95f;
    col.b = 0.55f;
  }else if(qaKeV >= -3000.0){
    col.r = 0.6f;
    col.g = 0.95f;
    col.b = 0.6f;
  }else if(qaKeV >= -3500.0){
    col.r = 0.55f;
    col.g = 1.0f;
    col.b = 0.65f;
  }else if(qaKeV >= -4000.0){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(qaKeV >= -5000.0){
    col.r = 0.4f;
    col.g = 0.95f;
    col.b = 0.7f;
  }else if(qaKeV >= -6000.0){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(qaKeV >= -7000.0){
    col.r = 0.3f;
    col.g = 0.8f;
    col.b = 0.7f;
  }else if(qaKeV >= -8000.0){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(qaKeV >= -9000.0){
    col.r = 0.3f;
    col.g = 0.6f;
    col.b = 0.8f;
  }else if(qaKeV >= -10000.0){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(qaKeV >= -11000.0){
    col.r = 0.25f;
    col.g = 0.45f;
    col.b = 0.8f;
  }else if(qaKeV >= -12000.0){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(qaKeV >= -14000.0){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(qaKeV >= -17000.0){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(qaKeV >= -20000.0){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else{
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }
  //slightly darken stable nuclides
  /*if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }*/
  return col;
}

SDL_FColor getQbCol(const double qbKeV){
  SDL_FColor col;
  col.a = 1.0f;
  if(qbKeV == 0.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 1.0f;
  }else if(qbKeV >= 18000.0){
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.9f;
  }else if(qbKeV >= 15000.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(qbKeV >= 12000.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(qbKeV >= 10000.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(qbKeV >= 8000.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(qbKeV >= 6000.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(qbKeV >= 4000.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(qbKeV >= 2000.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(qbKeV >= 1000.0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(qbKeV >= 0.0){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(qbKeV >= -1000.0){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(qbKeV >= -2000.0){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(qbKeV >= -4000.0){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(qbKeV >= -6000.0){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(qbKeV >= -8000.0){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(qbKeV >= -10000.0){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(qbKeV >= -12000.0){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(qbKeV >= -15000.0){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else{
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }
  //slightly darken stable nuclides
  /*if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }*/
  return col;
}

SDL_FColor getNumLvlsCol(const uint16_t numLvls, const double halflifeSeconds){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(numLvls >= 500){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(numLvls >= 450){
    col.r = 0.0f;
    col.g = 0.05f;
    col.b = 0.2f;
  }else if(numLvls >= 400){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(numLvls >= 350){
    col.r = 0.05f;
    col.g = 0.15f;
    col.b = 0.45f;
  }else if(numLvls >= 300){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(numLvls >= 275){
    col.r = 0.1f;
    col.g = 0.25f;
    col.b = 0.6f;
  }else if(numLvls >= 250){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(numLvls >= 225){
    col.r = 0.15f;
    col.g = 0.35f;
    col.b = 0.75f;
  }else if(numLvls >= 200){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(numLvls >= 175){
    col.r = 0.25f;
    col.g = 0.45f;
    col.b = 0.8f;
  }else if(numLvls >= 150){ //box color inversion point
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(numLvls >= 125){
    col.r = 0.3f;
    col.g = 0.6f;
    col.b = 0.75f;
  }else if(numLvls >= 100){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(numLvls >= 90){
    col.r = 0.3f;
    col.g = 0.8f;
    col.b = 0.7f;
  }else if(numLvls >= 80){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(numLvls >= 70){
    col.r = 0.4f;
    col.g = 0.95f;
    col.b = 0.7f;
  }else if(numLvls >= 60){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(numLvls >= 50){
    col.r = 0.6f;
    col.g = 0.95f;
    col.b = 0.6f;
  }else if(numLvls >= 40){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(numLvls >= 35){
    col.r = 0.8f;
    col.g = 0.9f;
    col.b = 0.45f;
  }else if(numLvls >= 30){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(numLvls >= 25){
    col.r = 0.95f;
    col.g = 0.95f;
    col.b = 0.35f;
  }else if(numLvls >= 20){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.3f;
  }else if(numLvls >= 15){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(numLvls >= 10){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(numLvls >= 7){
    col.r = 1.0f;
    col.g = 0.65f;
    col.b = 0.55f;
  }else if(numLvls >= 5){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(numLvls >= 4){
    col.r = 1.0f;
    col.g = 0.65f;
    col.b = 0.7f;
  }else if(numLvls >= 3){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(numLvls >= 2){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else{
    col.r = 1.0f;
    col.g = 0.9f;
    col.b = 0.9f;
  }
  //slightly darken stable nuclides
  if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }
  return col;
}

SDL_FColor getUnknownLvlsCol(const uint16_t unknownLvls, const double halflifeSeconds){
  SDL_FColor col;
  col.r = 0.9f;
  col.g = 0.9f;
  col.b = 0.9f;
  col.a = 1.0f;
  if(unknownLvls == 0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.9f;
  }else if(unknownLvls == 1){ //box color inversion point
    col.r = 0.4f;
    col.g = 0.6f;
    col.b = 0.9f;
  }else if(unknownLvls == 2){
    col.r = 0.4f;
    col.g = 0.9f;
    col.b = 0.6f;
  }else if(unknownLvls == 3){
    col.r = 0.8f;
    col.g = 0.9f;
    col.b = 0.2f;
  }else if(unknownLvls == 4){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.5f;
  }else if(unknownLvls == 5){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else{
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  //slightly darken stable nuclides
  if(halflifeSeconds > 1.0E15){
    col.r -= 0.1f;
    col.g -= 0.1f;
    col.b -= 0.1f;
  }
  return col;
}

uint8_t getNuclBoxLabelNumLines(const app_data *restrict dat, const uint16_t nuclInd){
  uint8_t numLines = 2;
  if(dat->ndat.nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
    numLines++;
  }
  if(dat->ndat.nuclData[nuclInd].numLevels > 0){
    uint32_t gsLevInd = (uint32_t)(dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel);
    if(dat->ndat.levels[gsLevInd].numDecModes > 0){
      numLines += (uint8_t)(dat->ndat.levels[gsLevInd].numDecModes);
    }
  }
  return numLines;
}

//draws label for the isomer spin box
void drawIsomerSpinBoxLabel(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint16_t nuclInd, const uint32_t isomerLvl, const uint8_t isomerMVal){
  char tmpStr[32];
  float drawXPos, drawYPos;
  const float labelMargin = NUCLBOX_LABEL_SMALLMARGIN*state->ds.chartZoomScale*state->ds.uiUserScale;
  const uint16_t Z = (uint16_t)dat->ndat.nuclData[nuclInd].Z;
  if(boxHeight > 38.0f){
    drawXPos = xPos + labelMargin;
    float totalLblHeight = getTextHeight(rdat,FONTSIZE_LARGE,getElemStr((uint8_t)Z))/rdat->uiDPIScale;
    drawYPos = yPos+boxHeight*0.5f - totalLblHeight*0.5f - 5.0f*state->ds.uiUserScale;
    getNuclNameStr(tmpStr,&dat->ndat.nuclData[nuclInd],(dat->ndat.nuclData[nuclInd].numIsomerMVals > 1) ? isomerMVal : 0);
    drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*state->ds.uiUserScale),col,FONTSIZE_LARGE,255,tmpStr,ALIGN_LEFT,16384); //draw element label
    //handle decay mode, spin-parity labels
    drawXPos = xPos + boxWidth - labelMargin;
    if(dat->ndat.levels[isomerLvl].numSpinParVals > 0){
      //spin-parity is known
      char jPiStr[64];
      getSpinParStr(tmpStr,&dat->ndat,isomerLvl);
      SDL_snprintf(jPiStr,64,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_JPI]],tmpStr);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,jPiStr,ALIGN_RIGHT,16384); //draw spin-parity label
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_RIGHT,16384); //draw unknown label
    }
  }
}

//draws label for the isomer decay mode box
void drawIsomerDecayModeBoxLabel(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint16_t nuclInd, const uint32_t isomerLvl, const uint8_t isomerMVal, const uint8_t dcyMode){
  char tmpStr[32];
  float drawXPos, drawYPos;
  const float labelMargin = NUCLBOX_LABEL_SMALLMARGIN*state->ds.chartZoomScale*state->ds.uiUserScale;
  const uint16_t Z = (uint16_t)dat->ndat.nuclData[nuclInd].Z;
  if(boxHeight > 38.0f){
    drawXPos = xPos + labelMargin;
    float totalLblHeight = getTextHeight(rdat,FONTSIZE_LARGE,getElemStr((uint8_t)Z))/rdat->uiDPIScale;
    drawYPos = yPos+boxHeight*0.5f - totalLblHeight*0.5f - 5.0f*state->ds.uiUserScale;
    getNuclNameStr(tmpStr,&dat->ndat.nuclData[nuclInd],(dat->ndat.nuclData[nuclInd].numIsomerMVals > 1) ? isomerMVal : 0);
    drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*state->ds.uiUserScale),col,FONTSIZE_LARGE,255,tmpStr,ALIGN_LEFT,16384); //draw element label
    float drawSpace = boxWidth - (10.0f*state->ds.uiUserScale) - getTextWidth(rdat,FONTSIZE_LARGE,tmpStr)/rdat->uiDPIScale;
    //handle decay mode, spin-parity labels
    drawXPos = xPos + boxWidth - labelMargin;
    if(dat->ndat.levels[isomerLvl].numSpinParVals > 0){
      //spin-parity is known
      getSpinParStr(tmpStr,&dat->ndat,isomerLvl);
      drawTextAlignedSized(rdat,drawXPos,drawYPos-(2.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw spin-parity label
      if(dcyMode >= DECAYMODE_ENUM_LENGTH){
        SDL_snprintf(tmpStr,32,"%s",dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
      }else{
        getMostProbableDecayModeStr(tmpStr,&dat->ndat,isomerLvl);
      }
      if(drawSpace > getTextWidth(rdat,FONTSIZE_LARGE,tmpStr)/rdat->uiDPIScale){
        drawTextAlignedSized(rdat,drawXPos,drawYPos+(18.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw decay mode label
      }else{
        drawTextAlignedSized(rdat,drawXPos,drawYPos+(18.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_RIGHT,16384);
      }
    }else{
      //only decay mode known
      if(dcyMode >= DECAYMODE_ENUM_LENGTH){
        SDL_snprintf(tmpStr,32,"%s",dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
      }else{
        getMostProbableDecayModeStr(tmpStr,&dat->ndat,isomerLvl);
      }
      if(drawSpace > getTextWidth(rdat,FONTSIZE_LARGE,tmpStr)/rdat->uiDPIScale){
        drawTextAlignedSized(rdat,drawXPos,drawYPos+(8.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw decay mode label
      }else{
        drawTextAlignedSized(rdat,drawXPos,drawYPos+(8.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_RIGHT,16384);
      }
    }
  }
}

//draws label for the isomer box
//isomerInd: which isomer is being drawn (m-number, eg. 178m1Hf vs. 178m2Hf)
void drawisomerBoxLabel(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint16_t nuclInd, const uint32_t isomerLvl, const uint8_t isomerMVal){
  char tmpStr[32];
  float drawXPos, drawYPos;
  const float labelMargin = NUCLBOX_LABEL_SMALLMARGIN*state->ds.chartZoomScale*state->ds.uiUserScale;
  const uint16_t Z = (uint16_t)dat->ndat.nuclData[nuclInd].Z;
  if(boxHeight > 38.0f){
    drawXPos = xPos + labelMargin;
    const float totalLblHeight = SDL_roundf(getTextHeight(rdat,FONTSIZE_LARGE,getElemStr((uint8_t)Z))/rdat->uiDPIScale);
    drawYPos = yPos+boxHeight*0.5f - totalLblHeight*0.5f - 5.0f*state->ds.uiUserScale;
    getNuclNameStr(tmpStr,&dat->ndat.nuclData[nuclInd],(dat->ndat.nuclData[nuclInd].numIsomerMVals > 1) ? isomerMVal : 0);
    drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*state->ds.uiUserScale),col,FONTSIZE_LARGE,255,tmpStr,ALIGN_LEFT,16384); //draw element label
    //handle half-life, spin-parity labels
    drawXPos = xPos + boxWidth - labelMargin;
    if(dat->ndat.levels[isomerLvl].numSpinParVals > 0){
      //spin-parity is known
      getSpinParStr(tmpStr,&dat->ndat,isomerLvl);
      drawTextAlignedSized(rdat,drawXPos,drawYPos-(2.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw spin-parity label
      getHalfLifeStr(tmpStr,dat,isomerLvl,1,0,state->ds.useLifetimes);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(18.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw half-life label
    }else{
      //only half-life known
      getHalfLifeStr(tmpStr,dat,isomerLvl,1,0,state->ds.useLifetimes);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(8.0f*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw half-life label
    }
  }
}

void drawNuclBoxLabelDetails(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint16_t nuclInd){
  char tmpStr[32];
  const uint32_t gsLevInd = (uint32_t)(dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel);
  uint8_t yOffsets = 0;
  uint8_t yOffsetLimit = 1;
  yOffsetLimit += (uint8_t)(floorf((boxHeight - 75.0f*state->ds.uiUserScale)/(30.0f*state->ds.uiUserScale)));
  //SDL_Log("Offset limit: %u\n",yOffsetLimit);
  const float labelMargin = NUCLBOX_LABEL_MARGIN*state->ds.chartZoomScale*state->ds.uiUserScale;
  const float labelSmallMargin = 2.0f*NUCLBOX_LABEL_SMALLMARGIN*state->ds.chartZoomScale*state->ds.uiUserScale;
  const Uint16 maxLblWidth = (Uint16)(boxWidth - 2.0f*labelMargin);
  const float drawXPos = xPos + boxWidth/2;
  const float drawYPos = yPos + 1.6f*labelMargin;
  uint8_t drawYOffsets = 1;
  if(state->chartView == CHARTVIEW_HALFLIFE){
    getGSHalfLifeStr(tmpStr,dat,nuclInd,state->ds.useLifetimes);
    if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
      drawYOffsets = 2;
    }
    if((yOffsets+drawYOffsets) <= yOffsetLimit){
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw GS half-life label
      yOffsets += drawYOffsets;
      if(dat->ndat.nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
        getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
        drawYOffsets = 1;
        if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
          drawYOffsets = 2;
        }
        if(((yOffsets+drawYOffsets) <= yOffsetLimit)||((drawYOffsets == 1)&&(dat->ndat.levels[gsLevInd].numDecModes==0))){
          //SDL_Log("Abundance y-offset: %u\n",yOffsets);
          drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw abundance label
          yOffsets += drawYOffsets;
          for(int8_t i=0; i<dat->ndat.levels[gsLevInd].numDecModes; i++){
            getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[gsLevInd].firstDecMode + (uint32_t)i);
            //SDL_Log("%s\n",tmpStr);
            drawYOffsets = 1;
            if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
              drawYOffsets = 2;
            }
            if(((yOffsets+drawYOffsets) <= yOffsetLimit)||((drawYOffsets == 1)&&(i==(dat->ndat.levels[gsLevInd].numDecModes-1)))){
              drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw decay mode label
              //SDL_Log("height: %f\n",(double)height);
              yOffsets += drawYOffsets;
            }else{
              drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
              break;
            }
          }
        }else{
          drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
        }
      }else{
        for(int8_t i=0; i<dat->ndat.levels[gsLevInd].numDecModes; i++){
          getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[gsLevInd].firstDecMode + (uint32_t)i);
          //SDL_Log("%s\n",tmpStr);
          drawYOffsets = 1;
          if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
            drawYOffsets = 2;
          }
          if(((yOffsets+drawYOffsets) <= yOffsetLimit)||((drawYOffsets == 1)&&(i==(dat->ndat.levels[gsLevInd].numDecModes-1)))){
            drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw decay mode label
            //SDL_Log("height: %f\n",(double)height);
            yOffsets += drawYOffsets;
          }else{
            drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
            break;
          }
        }
      }
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
    }
    //draw corner label(s)
    if(state->ds.chartZoomScale >= 18.0f){
      getSpinParStr(tmpStr,&dat->ndat,gsLevInd);
      drawTextAlignedSized(rdat,xPos+boxWidth-labelSmallMargin,yPos+labelSmallMargin,col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw spin-parity label
    }else if(dat->ndat.levels[gsLevInd].numSpinParVals <= 1){
      getSpinParStr(tmpStr,&dat->ndat,gsLevInd);
      drawTextAlignedSized(rdat,xPos+boxWidth-labelSmallMargin,yPos+labelSmallMargin,col,FONTSIZE_SMALL,255,tmpStr,ALIGN_RIGHT,16384); //draw small spin-parity label
    }else{
      drawTextAlignedSized(rdat,xPos+boxWidth-labelSmallMargin,yPos+labelSmallMargin,col,FONTSIZE_SMALL,255,"(...)",ALIGN_RIGHT,16384);
    }
  }else if(state->chartView == CHARTVIEW_DECAYMODE){
    //draw decay modes only
    if(dat->ndat.levels[gsLevInd].halfLife.unit == VALUE_UNIT_STABLE){
      //if the nuclide is stable, show the 'STABLE' label
      getGSHalfLifeStr(tmpStr,dat,nuclInd,state->ds.useLifetimes);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw GS half-life label
      yOffsets += drawYOffsets;
    }
    if(dat->ndat.nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
      getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
      drawYOffsets = 1;
      if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
        drawYOffsets = 2;
      }
      if(((yOffsets+drawYOffsets) <= yOffsetLimit)||((drawYOffsets == 1)&&(dat->ndat.levels[gsLevInd].numDecModes==0))){
        //SDL_Log("Abundance y-offset: %u\n",yOffsets);
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw abundance label
        yOffsets += drawYOffsets;
        for(int8_t i=0; i<dat->ndat.levels[gsLevInd].numDecModes; i++){
          getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[gsLevInd].firstDecMode + (uint32_t)i);
          //SDL_Log("%s\n",tmpStr);
          drawYOffsets = 1;
          if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (1.3f*maxLblWidth*rdat->uiDPIScale)){
            drawYOffsets = 3;
          }else if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
            drawYOffsets = 2;
          }
          if(((yOffsets+drawYOffsets) <= yOffsetLimit)||((drawYOffsets == 1)&&(i==(dat->ndat.levels[gsLevInd].numDecModes-1)))){
            drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw decay mode label
            //SDL_Log("height: %f\n",(double)height);
            yOffsets += drawYOffsets;
          }else{
            drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
            break;
          }
        }
      }else{
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
      }
    }else{
      for(int8_t i=0; i<dat->ndat.levels[gsLevInd].numDecModes; i++){
        getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[gsLevInd].firstDecMode + (uint32_t)i);
        //SDL_Log("%s\n",tmpStr);
        drawYOffsets = 1;
        if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (1.3f*maxLblWidth*rdat->uiDPIScale)){
          drawYOffsets = 3;
        }else if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
          drawYOffsets = 2;
        }
        if(((yOffsets+drawYOffsets) <= yOffsetLimit)||((drawYOffsets == 1)&&(i==(dat->ndat.levels[gsLevInd].numDecModes-1)))){
          drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw decay mode label
          //SDL_Log("height: %f\n",(double)height);
          yOffsets += drawYOffsets;
        }else{
          drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
          break;
        }
      }
    }
    //draw corner label(s)
    if(state->ds.chartZoomScale >= 18.0f){
      getSpinParStr(tmpStr,&dat->ndat,gsLevInd);
      drawTextAlignedSized(rdat,xPos+boxWidth-labelSmallMargin,yPos+labelSmallMargin,col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw spin-parity label
    }else if(dat->ndat.levels[gsLevInd].numSpinParVals <= 1){
      getSpinParStr(tmpStr,&dat->ndat,gsLevInd);
      drawTextAlignedSized(rdat,xPos+boxWidth-labelSmallMargin,yPos+labelSmallMargin,col,FONTSIZE_SMALL,255,tmpStr,ALIGN_RIGHT,16384); //draw small spin-parity label
    }else{
      drawTextAlignedSized(rdat,xPos+boxWidth-labelSmallMargin,yPos+labelSmallMargin,col,FONTSIZE_SMALL,255,"(...)",ALIGN_RIGHT,16384);
    }
  }else if(state->chartView == CHARTVIEW_2PLUS){
    uint32_t plus2Lvl = get2PlusLvlInd(&dat->ndat,nuclInd);
    if(plus2Lvl != MAXNUMLVLS){
      getLvlEnergyStr(tmpStr,&dat->ndat,plus2Lvl,1);
      SDL_strlcat(tmpStr," keV",32);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw 2+ energy label
    }
  }else if(state->chartView == CHARTVIEW_R42){
    double r42 = getR42(&dat->ndat,nuclInd);
    if(r42 > 0.0){
      SDL_snprintf(tmpStr,32,"R₄₂ = %0.2f",r42);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw R42 label
    }
  }else if(state->chartView == CHARTVIEW_BETA2){
    double beta2 = getBeta2(&dat->ndat,nuclInd);
    if(beta2 > 0.0){
      SDL_snprintf(tmpStr,32,"%s = %0.3f",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BETA2]],beta2);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw beta2 label
    }
  }else if(state->chartView == CHARTVIEW_SPIN){
    //draw spin-parity only
    if(getMostProbableSpin(&dat->ndat,gsLevInd) >= 255.0){
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_CENTER,maxLblWidth); //draw unknown label
    }else{
      char jPiStr[64];
      getSpinParStr(tmpStr,&dat->ndat,gsLevInd);
      SDL_snprintf(jPiStr,64,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_JPI]],tmpStr);
      drawYOffsets = 1;
      if(getTextWidth(rdat,FONTSIZE_NORMAL,jPiStr) > (maxLblWidth*rdat->uiDPIScale)){
        drawYOffsets = 2;
      }
      if((yOffsets+drawYOffsets) <= yOffsetLimit){
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,jPiStr,ALIGN_CENTER,maxLblWidth); //draw spin-parity label
        //SDL_Log("height: %f\n",(double)height);
        yOffsets += drawYOffsets;
      }else{
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
      }
    }
  }else if(state->chartView == CHARTVIEW_PARITY){
    //draw spin-parity only
    if(getMostProbableParity(&dat->ndat,gsLevInd) == 0){
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_CENTER,maxLblWidth); //draw unknown label
    }else{
      char jPiStr[64];
      getSpinParStr(tmpStr,&dat->ndat,gsLevInd);
      SDL_snprintf(jPiStr,64,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_JPI]],tmpStr);
      drawYOffsets = 1;
      if(getTextWidth(rdat,FONTSIZE_NORMAL,jPiStr) > (maxLblWidth*rdat->uiDPIScale)){
        drawYOffsets = 2;
      }
      if((yOffsets+drawYOffsets) <= yOffsetLimit){
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,jPiStr,ALIGN_CENTER,maxLblWidth); //draw spin-parity label
        //SDL_Log("height: %f\n",(double)height);
        yOffsets += drawYOffsets;
      }else{
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
      }
    }
  }else if(state->chartView == CHARTVIEW_BEA){
    double beA = getBEA(&dat->ndat,nuclInd);
    if(beA > 0.0){
      getMassValStr(tmpStr,dat->ndat.nuclData[nuclInd].beA,1);
      SDL_strlcat(tmpStr," keV",32);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw BE/A label
    }
  }else if(state->chartView == CHARTVIEW_SN){
    if(dat->ndat.nuclData[nuclInd].sn.val != 0.0f){
      getQValStr(tmpStr,dat->ndat.nuclData[nuclInd].sn,1);
      SDL_strlcat(tmpStr," keV",32);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw Sn label
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_CENTER,maxLblWidth); //draw Sn label
    }
  }else if(state->chartView == CHARTVIEW_SP){
    if(dat->ndat.nuclData[nuclInd].sp.val != 0.0f){
      getQValStr(tmpStr,dat->ndat.nuclData[nuclInd].sp,1);
      SDL_strlcat(tmpStr," keV",32);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw Sp label
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_CENTER,maxLblWidth); //draw Sp label
    }
  }else if(state->chartView == CHARTVIEW_QALPHA){
    if(dat->ndat.nuclData[nuclInd].qalpha.val != 0.0f){
      getQValStr(tmpStr,dat->ndat.nuclData[nuclInd].qalpha,1);
      SDL_strlcat(tmpStr," keV",32);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw Qalpha label
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_CENTER,maxLblWidth); //draw Qalpha label
    }
  }else if(state->chartView == CHARTVIEW_QBETAMINUS){
    if(dat->ndat.nuclData[nuclInd].qbeta.val != 0.0f){
      getQValStr(tmpStr,dat->ndat.nuclData[nuclInd].qbeta,1);
      SDL_strlcat(tmpStr," keV",32);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw Qbeta label
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 40.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_CENTER,maxLblWidth); //draw Qbeta label
    }
  }else if(state->chartView == CHARTVIEW_NUMLVLS){
    SDL_snprintf(tmpStr,32,"%u",dat->ndat.nuclData[nuclInd].numLevels);
    if(dat->ndat.nuclData[nuclInd].numLevels == 1){
      SDL_strlcat(tmpStr," level",32);
    }else{
      SDL_strlcat(tmpStr," levels",32);
    }
    drawYOffsets = 1;
    if((yOffsets+drawYOffsets) <= yOffsetLimit){
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw number of known levels label      
      //SDL_Log("height: %f\n",(double)height);
      yOffsets += drawYOffsets;
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
    }
    if((yOffsets+drawYOffsets) <= yOffsetLimit){
      if(dat->ndat.levels[gsLevInd].halfLife.unit == VALUE_UNIT_STABLE){
        //if the nuclide is stable, show the 'STABLE' label
        getGSHalfLifeStr(tmpStr,dat,nuclInd,state->ds.useLifetimes);
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw GS half-life label
      }
    }
  }else if(state->chartView == CHARTVIEW_UNKNOWN_ENERGY){
    const uint16_t numUnknowns = getNumUnknownLvls(&dat->ndat,nuclInd);
    SDL_snprintf(tmpStr,32,"%u",numUnknowns);
    if(numUnknowns == 1){
      SDL_strlcat(tmpStr," level",32);
    }else{
      SDL_strlcat(tmpStr," levels",32);
    }
    drawYOffsets = 1;
    if((yOffsets+drawYOffsets) <= yOffsetLimit){
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw number of unknown levels label      
      //SDL_Log("height: %f\n",(double)height);
      yOffsets += drawYOffsets;
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_CENTER,maxLblWidth);
    }
    if((yOffsets+drawYOffsets) <= yOffsetLimit){
      if(dat->ndat.levels[gsLevInd].halfLife.unit == VALUE_UNIT_STABLE){
        //if the nuclide is stable, show the 'STABLE' label
        getGSHalfLifeStr(tmpStr,dat,nuclInd,state->ds.useLifetimes);
        drawTextAlignedSized(rdat,drawXPos,drawYPos+((yOffsets*19.0f + 36.0f)*state->ds.uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,maxLblWidth); //draw GS half-life label
      }
    }
  }
  
}

//draws the main label for a box on the chart of nuclides
void drawNuclBoxLabel(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, const SDL_Color col, const uint16_t nuclInd){
  char tmpStr[32];
  float drawXPos, drawYPos;
  const float labelMargin = NUCLBOX_LABEL_MARGIN*state->ds.chartZoomScale*state->ds.uiUserScale;
  const uint16_t Z = (uint16_t)dat->ndat.nuclData[nuclInd].Z;
  Uint8 alpha = 255;
  if(state->ds.chartZoomScale < 4.0f){
    //handle fade-in of text
    alpha = (Uint8)((1.0f - 2.0f*(4.0f - state->ds.chartZoomScale))*255.0f);
  }
  if(state->ds.chartZoomScale >= 7.3f){
    getNuclNameStr(tmpStr,&dat->ndat.nuclData[nuclInd],255);
    const float totalLblWidth = SDL_roundf(getTextWidth(rdat,FONTSIZE_LARGE,tmpStr)/rdat->uiDPIScale);
    drawXPos = xPos+boxWidth*0.5f - totalLblWidth*0.5f;
    if(state->ds.chartZoomScale >= 12.0f){
      drawYPos = yPos + labelMargin;
    }else{
      const float totalLblHeight = SDL_roundf((getTextHeight(rdat,FONTSIZE_SMALL,tmpStr) - (2.0f*state->ds.uiUserScale) + getTextHeight(rdat,FONTSIZE_LARGE,getElemStr((uint8_t)Z)))/rdat->uiDPIScale);
      drawYPos = yPos+boxWidth*0.5f - totalLblHeight*0.5f;
    }
    drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*state->ds.uiUserScale),col,FONTSIZE_LARGE,alpha,tmpStr,ALIGN_LEFT,16384); //draw number and element label
    if(state->ds.chartZoomScale >= 12.0f){
      drawNuclBoxLabelDetails(dat,state,rdat,xPos,yPos,boxWidth,boxHeight,col,nuclInd);
    }
  }else{
    drawTextAlignedSized(rdat,xPos+boxWidth*0.5f,yPos+boxWidth*0.5f,col,FONTSIZE_NORMAL,alpha,getElemStr((uint8_t)Z),ALIGN_CENTER,16384); //draw element label only
  }
}

void drawChartOfNuclides(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){

  float minX = getMinChartN(&state->ds);
  float maxX = getMaxChartN(&state->ds);
  float minY = getMinChartZ(&state->ds);
  float maxY = getMaxChartZ(&state->ds);
  
  //SDL_Log("N range: [%0.2f %0.2f], Z range: [%0.2f %0.2f]\n",(double)minX,(double)maxX,(double)minY,(double)maxY);

  SDL_FRect rect, lowBoxRect;
  float nuclBoxWidth = DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale;
  float lowBoxPadding = DEFAULT_LOWBOX_PADDING*state->ds.chartZoomScale*state->ds.uiUserScale;
  float boxLineLimit = powf(state->ds.chartZoomScale/MAX_CHART_ZOOM_SCALE,2.0f)*9.0f;

  //calculate size of low box (extra info like isomers, 2+ energies etc.)
  float lowBoxHeight = 0.0f;
  if(state->ds.chartZoomScale >= 4.5f){
    lowBoxHeight = (boxLineLimit - 3)*20.0f*state->ds.uiUserScale;
    if(lowBoxHeight < 0.0f){
      lowBoxHeight = 0.0f;
    }
    lowBoxHeight += 1.5f*DEFAULT_LOWBOX_PADDING*state->ds.chartZoomScale*state->ds.uiUserScale; //minimum size
    if(lowBoxHeight > (80.0f*state->ds.uiUserScale)){
      lowBoxHeight = 80.0f*state->ds.uiUserScale; //limit low box size
    }
  }
  

  //printf("line limit: %0.3f\n",(double)boxLineLimit);
  for(uint16_t i=0;i<dat->ndat.numNucl;i++){
    if((dat->ndat.nuclData[i].flags & 3U) == OBSFLAG_OBSERVED){
      if((dat->ndat.nuclData[i].N >= (int16_t)SDL_floorf(minX))&&(dat->ndat.nuclData[i].N >= 0)){
        if(dat->ndat.nuclData[i].N <= (int16_t)SDL_ceilf(maxX)){
          if((dat->ndat.nuclData[i].Z >= (int16_t)SDL_floorf(minY))&&(dat->ndat.nuclData[i].Z >= 0)){
            if(dat->ndat.nuclData[i].Z <= (int16_t)SDL_ceilf(maxY)){
              //draw nuclide on chart
              rect.w = nuclBoxWidth;
              rect.h = rect.w;
              rect.x = ((float)dat->ndat.nuclData[i].N - minX)*rect.w;
              rect.y = (maxY - (float)dat->ndat.nuclData[i].Z)*rect.h;
              //SDL_Log("N: %i, Z: %i, i: %i, pos: [%0.2f %0.2f %0.2f %0.2f]\n",dat->ndat.nuclData[i].N,dat->ndat.nuclData[i].Z,i,(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
              //const double hl = getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i);
              SDL_FColor boxCol = {0.0f,0.0f,0.0f,1.0f};
              if(state->chartView == CHARTVIEW_HALFLIFE){
                boxCol = getHalfLifeCol(getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i));
              }else if(state->chartView == CHARTVIEW_DECAYMODE){
                boxCol = getDecayModeCol(getNuclGSMostProbableDcyMode(&dat->ndat,(uint16_t)i));
              }else if(state->chartView == CHARTVIEW_2PLUS){
                boxCol = get2PlusCol(get2PlusEnergy(&dat->ndat,(uint16_t)i),getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i));
              }else if(state->chartView == CHARTVIEW_R42){
                boxCol = getR42Col(getR42(&dat->ndat,(uint16_t)i),getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i));
              }else if(state->chartView == CHARTVIEW_BETA2){
                boxCol = getBeta2Col(getBeta2(&dat->ndat,(uint16_t)i),getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i));
              }else if(state->chartView == CHARTVIEW_SPIN){
                boxCol = getSpinCol(getMostProbableSpin(&dat->ndat,dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel));
              }else if(state->chartView == CHARTVIEW_PARITY){
                boxCol = getParCol(getMostProbableParity(&dat->ndat,dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel));
              }else if(state->chartView == CHARTVIEW_BEA){
                boxCol = getBEACol(getBEA(&dat->ndat,(uint16_t)i));
              }else if(state->chartView == CHARTVIEW_SN){
                boxCol = getSnpCol(getRawValFromDB(&dat->ndat.nuclData[i].sn));
              }else if(state->chartView == CHARTVIEW_SP){
                boxCol = getSnpCol(getRawValFromDB(&dat->ndat.nuclData[i].sp));
              }else if(state->chartView == CHARTVIEW_QALPHA){
                boxCol = getQaCol(getRawValFromDB(&dat->ndat.nuclData[i].qalpha));
              }else if(state->chartView == CHARTVIEW_QBETAMINUS){
                boxCol = getQbCol(getRawValFromDB(&dat->ndat.nuclData[i].qbeta));
              }else if(state->chartView == CHARTVIEW_NUMLVLS){
                boxCol = getNumLvlsCol(dat->ndat.nuclData[i].numLevels,getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i));
              }else if(state->chartView == CHARTVIEW_UNKNOWN_ENERGY){
                boxCol = getUnknownLvlsCol(getNumUnknownLvls(&dat->ndat,(uint16_t)i),getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i));
              }
              drawFlatRect(rdat,rect,boxCol);
              
              if(state->ds.chartZoomScale >= 3.5f){
                uint8_t drawingLowBox = 0;
                if(state->ds.chartZoomScale >= 4.5f){
                  //setup low box rect
                  lowBoxRect.x = rect.x + lowBoxPadding;
                  lowBoxRect.y = rect.y + rect.h - lowBoxHeight - lowBoxPadding;
                  lowBoxRect.w = nuclBoxWidth - 2.0f*lowBoxPadding;
                  lowBoxRect.h = lowBoxHeight;
                  if(state->chartView == CHARTVIEW_HALFLIFE){
                    //draw isomer box
                    uint32_t isomerLvl = dat->ndat.nuclData[i].longestIsomerLevel;
                    const double isomerHl = getLevelHalfLifeSeconds(&dat->ndat,isomerLvl);
                    if((isomerLvl != MAXNUMLVLS)&&(isomerLvl != (dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel))){
                      if((isomerHl >= 1.0E-1)||(isomerHl > getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i))){ //only show 'important' isomers on chart
                        drawingLowBox = 1;
                        SDL_FColor iboxCol = getHalfLifeCol(isomerHl);
                        if((iboxCol.r == boxCol.r)&&(iboxCol.g == iboxCol.g)&&(iboxCol.b == boxCol.b)){
                          //make isomer box colors slightly different, to distinguish them from
                          //ground states of similar half-life
                          if(getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i) > isomerHl){
                            iboxCol.r *= 1.10f;
                            iboxCol.g *= 1.10f;
                            iboxCol.b *= 1.10f;
                          }else{
                            iboxCol.r *= 0.91f;
                            iboxCol.g *= 0.91f;
                            iboxCol.b *= 0.91f;
                          }
                        }
                        if(state->ds.chartZoomScale < 5.5f){
                          //handle fading in of isomer boxes
                          iboxCol.a =  1.0f - (5.5f-state->ds.chartZoomScale);
                        }
                        drawFlatRect(rdat,lowBoxRect,iboxCol);
                        drawisomerBoxLabel(dat,state,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,(isomerHl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i,isomerLvl,dat->ndat.nuclData[i].longestIsomerMVal);
                      }
                    }
                  }else if(state->chartView == CHARTVIEW_DECAYMODE){
                    //draw decay mode box
                    uint32_t isomerLvl = dat->ndat.nuclData[i].longestIsomerLevel;
                    const double isomerHl = getLevelHalfLifeSeconds(&dat->ndat,isomerLvl);
                    if((isomerLvl != MAXNUMLVLS)&&(isomerLvl != (dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel))){
                      if((isomerHl >= 1.0E-1)||(isomerHl > getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i))){ //only show 'important' isomers on chart
                        uint8_t isomerDcyMode = getLevelMostProbableDcyMode(&dat->ndat,dat->ndat.nuclData[i].longestIsomerLevel);
                        if(isomerDcyMode < (DECAYMODE_ENUM_LENGTH+1)){
                          drawingLowBox = 1;
                          SDL_FColor iboxCol = getDecayModeCol(isomerDcyMode);
                          if((iboxCol.r == boxCol.r)&&(iboxCol.g == iboxCol.g)&&(iboxCol.b == boxCol.b)){
                            //make isomer box colors slightly different, to distinguish them from
                            //ground states of the same decay mode
                            iboxCol.r *= 1.20f;
                            iboxCol.g *= 1.20f;
                            iboxCol.b *= 1.20f;
                          }
                          if(state->ds.chartZoomScale < 5.5f){
                            //handle fading in of isomer boxes
                            iboxCol.a =  1.0f - (5.5f-state->ds.chartZoomScale);
                          }
                          drawFlatRect(rdat,lowBoxRect,iboxCol);
                          drawIsomerDecayModeBoxLabel(dat,state,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,getDecayModeTextCol(isomerDcyMode),(uint16_t)i,isomerLvl,dat->ndat.nuclData[i].longestIsomerMVal,isomerDcyMode);
                        }else if(isomerHl > 1.0E15){
                          //'stable' isomer with no known decay mode
                          //draw its box using the half-life color
                          drawingLowBox = 1;
                          SDL_FColor iboxCol = getHalfLifeCol(isomerHl);
                          if((iboxCol.r == boxCol.r)&&(iboxCol.g == iboxCol.g)&&(iboxCol.b == boxCol.b)){
                            //make isomer box colors slightly different, to distinguish them from
                            //ground states of the same decay mode
                            iboxCol.r += 0.1f;
                            iboxCol.g += 0.1f;
                            iboxCol.b += 0.1f;
                          }
                          if(state->ds.chartZoomScale < 5.5f){
                            //handle fading in of isomer boxes
                            iboxCol.a =  1.0f - (5.5f-state->ds.chartZoomScale);
                          }
                          drawFlatRect(rdat,lowBoxRect,iboxCol);
                          drawIsomerDecayModeBoxLabel(dat,state,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,getDecayModeTextCol(isomerDcyMode),(uint16_t)i,isomerLvl,dat->ndat.nuclData[i].longestIsomerMVal,isomerDcyMode);
                        }
                      }
                    }
                  }else if(state->chartView == CHARTVIEW_SPIN){
                    //draw spin box
                    uint32_t isomerLvl = dat->ndat.nuclData[i].longestIsomerLevel;
                    const double isomerHl = getLevelHalfLifeSeconds(&dat->ndat,isomerLvl);
                    if((isomerLvl != MAXNUMLVLS)&&(isomerLvl != (dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel))){
                      if((isomerHl >= 1.0E-1)||(isomerHl > getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i))){ //only show 'important' isomers on chart
                        double isomerSpin = getMostProbableSpin(&dat->ndat,dat->ndat.nuclData[i].longestIsomerLevel);
                        drawingLowBox = 1;
                        SDL_FColor iboxCol = getSpinCol(isomerSpin);
                        if(state->ds.chartZoomScale < 5.5f){
                          //handle fading in of isomer boxes
                          iboxCol.a =  1.0f - (5.5f-state->ds.chartZoomScale);
                        }
                        drawFlatRect(rdat,lowBoxRect,iboxCol);
                        drawIsomerSpinBoxLabel(dat,state,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,(isomerSpin <= 2.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i,isomerLvl,dat->ndat.nuclData[i].longestIsomerMVal);
                      }
                    }
                  }else if(state->chartView == CHARTVIEW_PARITY){
                    //draw spin box
                    uint32_t isomerLvl = dat->ndat.nuclData[i].longestIsomerLevel;
                    const double isomerHl = getLevelHalfLifeSeconds(&dat->ndat,isomerLvl);
                    if((isomerLvl != MAXNUMLVLS)&&(isomerLvl != (dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel))){
                      if((isomerHl >= 1.0E-1)||(isomerHl > getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i))){ //only show 'important' isomers on chart
                        int8_t isomerPar = getMostProbableParity(&dat->ndat,dat->ndat.nuclData[i].longestIsomerLevel);
                        drawingLowBox = 1;
                        SDL_FColor iboxCol = getParCol(isomerPar);
                        if(state->ds.chartZoomScale < 5.5f){
                          //handle fading in of isomer boxes
                          iboxCol.a =  1.0f - (5.5f-state->ds.chartZoomScale);
                        }
                        drawFlatRect(rdat,lowBoxRect,iboxCol);
                        drawIsomerSpinBoxLabel(dat,state,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,(isomerPar < 0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i,isomerLvl,dat->ndat.nuclData[i].longestIsomerMVal);
                      }
                    }
                  }
                }
                if(drawingLowBox){
                  if(state->chartView == CHARTVIEW_HALFLIFE){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),(getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i) > 1.0E3) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_DECAYMODE){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),getDecayModeTextCol(getNuclGSMostProbableDcyMode(&dat->ndat,(uint16_t)i)),(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_SPIN){
                    double spin = getMostProbableSpin(&dat->ndat,dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel);
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),(spin <= 2.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_PARITY){
                    int8_t par = getMostProbableParity(&dat->ndat,dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel);
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),(par < 0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_BEA){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),(getBEA(&dat->ndat,(uint16_t)i) >= 8000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }
                }else{
                  if(state->chartView == CHARTVIEW_HALFLIFE){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i) > 1.0E3) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_DECAYMODE){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,getDecayModeTextCol(getNuclGSMostProbableDcyMode(&dat->ndat,(uint16_t)i)),(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_2PLUS){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(get2PlusEnergy(&dat->ndat,(uint16_t)i) >= 3000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_R42){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getR42(&dat->ndat,(uint16_t)i) >= 3.2) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_BETA2){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getBeta2(&dat->ndat,(uint16_t)i) >= 0.35) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_SPIN){
                    double spin = getMostProbableSpin(&dat->ndat,dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel);
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(spin <= 2.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_PARITY){
                    int8_t par = getMostProbableParity(&dat->ndat,dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel);
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(par < 0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_BEA){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getBEA(&dat->ndat,(uint16_t)i) >= 8000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_SN){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getRawValFromDB(&dat->ndat.nuclData[i].sn) >= 8000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_SP){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getRawValFromDB(&dat->ndat.nuclData[i].sp) >= 8000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_QALPHA){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getRawValFromDB(&dat->ndat.nuclData[i].qalpha) <= -10000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_QBETAMINUS){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(getRawValFromDB(&dat->ndat.nuclData[i].qbeta) <= -3000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_NUMLVLS){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,(dat->ndat.nuclData[i].numLevels >= 200) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_UNKNOWN_ENERGY){
                    drawNuclBoxLabel(dat,state,rdat,rect.x,rect.y,rect.w,rect.h,blackCol8Bit,(uint16_t)i);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  //draw shell closure lines
  //some fudging of line positions was needed to fix alignment with fractional scaling
  if(state->ds.drawShellClosures){
    SDL_FColor scCol = darkGrayCol;
    scCol.a = (state->ds.chartZoomScale/0.5f) - 3.0f;
    if(scCol.a > 0.0f){
      if(scCol.a > 1.0f){
        scCol.a = 1.0f;
      }
      for(uint8_t i=0; i<NUMSHELLCLOSURES; i++){
        //neutron shell closures
        if(shellClosureValues[i] <= dat->ndat.maxN){
          if(shellClosureValues[i] >= (minX - 1.0f)){
            if(shellClosureValues[i] <= (maxX + 1.0f)){
              if(dat->ndat.minZforN[shellClosureValues[i]] > minY){
                if(dat->ndat.maxZforN[shellClosureValues[i]] < maxY){
                  //top and bottom of shell closure are both on screen
                  drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }else{
                  //bottom of shell closure is on screen
                  drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }
              }else if(dat->ndat.maxZforN[shellClosureValues[i]] < maxY){
                //top of shell closure is on screen
                drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }else{
                //top and bottom of shell closure are both off screen
                drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }
            }
          }
        }
        //proton shell closures
        if(shellClosureValues[i] <= dat->ndat.maxZ){
          if(shellClosureValues[i] >= (minY - 1.0f)){
            if(shellClosureValues[i] <= (maxY + 1.0f)){
              if(dat->ndat.minNforZ[shellClosureValues[i]] > minX){
                if(dat->ndat.maxNforZ[shellClosureValues[i]] < maxX){
                  //left and right edges of shell closure are both on screen
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }else{
                  //left edge of shell closure is on screen
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }
              }else if(dat->ndat.maxNforZ[shellClosureValues[i]] < maxX){
                //right edge of shell closure is on screen
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }else{
                //left and right edges of shell closure are both off screen
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }
            }
          }
        }
      }
    }
  }
  

  //handle highlighting selected nuclide
  if(state->chartSelectedNucl != MAXNUMNUCL){
    float alpha = 1.0f;
    if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLHIGHLIGHT_HIDE)){
      alpha = (float)(juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLHIGHLIGHT_HIDE]/UI_ANIM_LENGTH));
    }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLHIGHLIGHT_SHOW)){
      alpha = (float)(juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLHIGHLIGHT_SHOW]/UI_ANIM_LENGTH));
    }
    SDL_FColor selectionCol = whiteCol;
    selectionCol.a = alpha;
    if(state->ds.chartZoomScale >= 2.0f){
      rect.x = ((float)dat->ndat.nuclData[state->chartSelectedNucl].N - minX)*nuclBoxWidth - CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale;
      rect.w =  nuclBoxWidth + 2.0f*CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale;
      rect.y = (maxY - (float)dat->ndat.nuclData[state->chartSelectedNucl].Z)*nuclBoxWidth - CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale;
      rect.h = rect.w;
      drawSelectionRect(rdat,rect,selectionCol,2.0f*CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale);
    }else{
      rect.x = ((float)dat->ndat.nuclData[state->chartSelectedNucl].N - minX)*nuclBoxWidth;
      rect.y = (maxY - (float)dat->ndat.nuclData[state->chartSelectedNucl].Z)*nuclBoxWidth;
      drawFlatRect(rdat,rect,selectionCol);
    }
  }

  //draw x and y axes
  rect.w = state->ds.windowXRes;
  rect.h = CHART_AXIS_DEPTH*state->ds.uiUserScale;
  rect.x = 0;
  rect.y = state->ds.windowYRes - rect.h;
  if(rdat->ssdat.takingScreenshot != 1){
    drawFlatRect(rdat,rect,whiteTransparentCol);
  }else{
    drawFlatRect(rdat,rect,whiteCol);
  }
  rect.w = rect.h;
  rect.h = rect.y;
  rect.y = 0;
  if(rdat->ssdat.takingScreenshot != 1){
    drawFlatRect(rdat,rect,whiteTransparentCol);
  }else{
    drawFlatRect(rdat,rect,whiteCol);
  }
  drawTextAligned(rdat,CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_LARGE,"Z",ALIGN_CENTER);
  drawTextAligned(rdat,state->ds.windowXRes - CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,state->ds.windowYRes - CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_LARGE,"N",ALIGN_CENTER);
  //draw ticks
  char tmpStr[32];
  rect.y = state->ds.windowYRes - (CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale);
  float tickSpacing = getAxisTickSpacing(state->ds.chartZoomScale);
  for(float i=(minX-fmodf(minX,tickSpacing));i<maxX;i+=tickSpacing){
    if(i >= 0.0f){
      uint16_t numInd = (uint16_t)(SDL_floorf(i));
      if((i<MAX_MASS_NUM)&&(i<=MAX_NEUTRON_NUM)){
        rect.x = (i + 0.5f - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale;
        if(rect.x < (state->ds.windowXRes - CHART_AXIS_DEPTH*state->ds.uiUserScale)){ //dodge axis label
          SDL_snprintf(tmpStr,32,"%u",numInd); //is this slow?
          drawTextAlignedSized(rdat,rect.x,rect.y,blackCol8Bit,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,16384); //draw number label
        }
      }
    }
  }
  rect.x = CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale;
  for(float i=(minY-fmodf(minY,tickSpacing));i<maxY;i+=tickSpacing){
    if(i >= 0.0f){
      uint16_t numInd = (uint16_t)(SDL_floorf(i));
      if((i<MAX_MASS_NUM)&&(i<MAX_PROTON_NUM)){
        rect.y = (maxY - i + 0.5f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale;
        if(rect.y > CHART_AXIS_DEPTH*state->ds.uiUserScale){ //dodge axis label
          SDL_snprintf(tmpStr,32,"%u",numInd); //is this slow?
          drawTextAlignedSized(rdat,rect.x,rect.y,blackCol8Bit,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,16384); //draw number label
        }
      }
    }
  }

}

uint8_t getHighlightState(const app_state *restrict state, const uint8_t uiElem){
  if(state->clickedUIElem == uiElem){
    return HIGHLIGHT_SELECTED;
  }
  if(state->mouseoverElement==uiElem){
    if(state->mouseholdElement==uiElem){
      return HIGHLIGHT_SELECTED;
    }else{
      return HIGHLIGHT_MOUSEOVER;
    }
  }else{
    if(state->mouseholdElement==uiElem){
      return HIGHLIGHT_SELECTED;
    }
    return HIGHLIGHT_NORMAL;
  }
}

void drawInfoBoxHeader(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float x, const float y, const Uint8 alpha, const uint16_t nuclInd){
  char tmpStr[32], nuclStr[32];
  float drawXPos = (float)(x + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale);
  float drawYPos = (float)(y + (PANEL_EDGE_SIZE + 1.5f*UI_PADDING_SIZE)*state->ds.uiUserScale);
  uint16_t nucA = (uint16_t)(dat->ndat.nuclData[nuclInd].Z + dat->ndat.nuclData[nuclInd].N);
  getNuclNameStr(nuclStr,&dat->ndat.nuclData[nuclInd],255);
  if((uint16_t)(dat->ndat.nuclData[nuclInd].Z <= 1)&&(dat->ndat.nuclData[nuclInd].N <= 2)){
    SDL_snprintf(tmpStr,32,"%s (%s)",nuclStr,getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,(uint8_t)dat->ndat.nuclData[nuclInd].N));
  }else if((uint16_t)(dat->ndat.nuclData[nuclInd].Z == 0)&&(dat->ndat.nuclData[nuclInd].N <= 5)){
    SDL_snprintf(tmpStr,32,"%s (%s)",nuclStr,getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,(uint8_t)dat->ndat.nuclData[nuclInd].N));
  }else{
    SDL_snprintf(tmpStr,32,"%s (%s-%u)",nuclStr,getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,(uint8_t)dat->ndat.nuclData[nuclInd].N),nucA);
  }
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos+(10.0f*state->ds.uiUserScale),blackCol8Bit,FONTSIZE_LARGE,alpha,tmpStr,ALIGN_LEFT,16384); //draw element label
}

void drawNuclFullInfoBox(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t nuclInd){

  float txtYOffset = 0.0f;
  Uint8 txtAlpha = 255;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_TXTFADEOUT)){
    txtYOffset = (80.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEOUT]/SHORT_UI_ANIM_LENGTH));
    txtAlpha = (uint8_t)(255.0f*juice_smoothStop2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEOUT]/SHORT_UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_TXTFADEIN)){
    txtYOffset = (80.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEIN]/UI_ANIM_LENGTH));
    txtAlpha = (uint8_t)(255.0f*juice_smoothStart2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEIN]/UI_ANIM_LENGTH));
  }
  
  //level and gamma data
  char tmpStr[32];
  SDL_FRect rect;
  SDL_FColor dividerLineCol = lightGrayCol;
  dividerLineCol.a = txtAlpha/255.0f;
  uint8_t drawMode = LLDRAWMODE_ALL;
  float allColWidth = state->ds.fullInfoAllColWidth*state->ds.uiUserScale;
  if(allColWidth > (state->ds.windowXRes - 4*UI_PADDING_SIZE*state->ds.uiUserScale)){
    drawMode = LLDRAWMODE_SKIPICC;
    allColWidth = state->ds.fullInfoAllColWidthExclICC*state->ds.uiUserScale;
    if(allColWidth > (state->ds.windowXRes - 4*UI_PADDING_SIZE*state->ds.uiUserScale)){
      drawMode = LLDRAWMODE_SKIPICC_MULT;
      allColWidth = state->ds.fullInfoAllColWidthExclICCM*state->ds.uiUserScale;
      if(allColWidth > (state->ds.windowXRes - 4*UI_PADDING_SIZE*state->ds.uiUserScale)){
        drawMode = LLDRAWMODE_SKIPICC_MULT_JF;
        allColWidth = state->ds.fullInfoAllColWidthExcluICCMFinalJpi*state->ds.uiUserScale;
      }
    }
  }else if(!(dat->ndat.nuclData[nuclInd].flags & (1U << 2))){
    //skip drawing ICC column as no data is available
    drawMode = LLDRAWMODE_SKIPICC;
  }
  float origDrawXPos = (state->ds.windowXRes - allColWidth)/2.0f;
  float drawXPos = origDrawXPos;
  float drawYPos = (NUCL_FULLINFOBOX_LEVELLIST_POS_Y - NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.nuclFullInfoScrollY)*state->ds.uiUserScale + txtYOffset;
  rect.x = origDrawXPos - 2*UI_PADDING_SIZE*state->ds.uiUserScale;
  rect.w = allColWidth + 4*UI_PADDING_SIZE*state->ds.uiUserScale;
  //SDL_Log("drawYPos: %f\n",(double)drawYPos);
  float levelStartDrawPos;
  for(uint32_t lvlInd = dat->ndat.nuclData[nuclInd].firstLevel; lvlInd<(dat->ndat.nuclData[nuclInd].firstLevel+dat->ndat.nuclData[nuclInd].numLevels); lvlInd++){
    
    //skip all levels which are not part of the selected reaction
    //SDL_Log("Lvl %u populating rxns: %lu\n",lvlInd,dat->ndat.levels[lvlInd].populatingRxns);
    if(state->ds.selectedRxn > 0){
      if(!(dat->ndat.levels[lvlInd].populatingRxns & ((uint64_t)(1) << (state->ds.selectedRxn-1)))){
        continue;
      }
    }

    drawXPos = origDrawXPos;
    uint16_t numLines = getNumDispLinesForLvl(&dat->ndat,lvlInd);
    if(((drawYPos + NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale*numLines) >= NUCL_FULLINFOBOX_LEVELLIST_POS_Y)&&(drawYPos <= state->ds.windowYRes)){
      
      const double hl = getLevelHalfLifeSeconds(&dat->ndat,lvlInd);
      if(hl > 1.0E-9){
        //highlight isomers and stable states
        //first check that the lifetime is not an upper limit
        if((((dat->ndat.levels[lvlInd].halfLife.format >> 5U) & 15U) != VALUETYPE_LESSTHAN)&&(((dat->ndat.levels[lvlInd].halfLife.format >> 5U) & 15U) != VALUETYPE_LESSOREQUALTHAN)){
          
          rect.h = (NUCL_INFOBOX_SMALLLINE_HEIGHT*numLines)*state->ds.uiUserScale;
          rect.y = drawYPos;
          SDL_FColor rectCol = getHalfLifeCol(hl);
          if(txtAlpha != 255){
            rectCol.a *= txtAlpha/255.0f;
          }
          drawFlatRect(rdat,rect,rectCol);
        }
        
      }
      
      levelStartDrawPos = drawYPos;
      if(lvlInd == (dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel)){
        getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,0);
      }else{
        getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,1);
      }
      drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384);
      //handle special level info
      const uint8_t slInd = (uint8_t)((dat->ndat.levels[lvlInd].format >> 1U) & 15U);
      const uint8_t mValInd = (uint8_t)((dat->ndat.levels[lvlInd].format >> 5U) & 7U);
      if(mValInd > 0){
        //handle isomer m-value labels
        drawYPos += (NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale + txtYOffset);
        char mValStr[32];
        if(dat->ndat.nuclData[nuclInd].numIsomerMVals > 1){
          getNuclNameStr(mValStr,&dat->ndat.nuclData[nuclInd],mValInd);
        }else{
          getNuclNameStr(mValStr,&dat->ndat.nuclData[nuclInd],0);
        }
        drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,mValStr,ALIGN_LEFT,16384);
      }
      if(slInd > 0){
        drawYPos += (NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale + txtYOffset);
        char slStr[64];
        SDL_snprintf(slStr,64,"%s",getSpecialLvlStr(dat,slInd));
        //SDL_Log("%s\n",tmpStr);
        drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,slStr,ALIGN_LEFT,16384);
      }
      drawYPos = levelStartDrawPos;
      drawXPos += state->ds.fullInfoElevelColWidth*state->ds.uiUserScale;
      getSpinParStr(tmpStr,&dat->ndat,lvlInd);
      drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384);
      drawXPos += state->ds.fullInfoJpiColWidth*state->ds.uiUserScale;
      getHalfLifeStr(tmpStr,dat,lvlInd,1,0,state->ds.useLifetimes);
      drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384);
      if(dat->ndat.levels[lvlInd].numDecModes > 0){
        for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
          drawYPos += (NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale + txtYOffset);
          getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
          //SDL_Log("%s\n",tmpStr);
          drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+(2*UI_PADDING_SIZE*state->ds.uiUserScale),drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw decay mode label
        }
      }
      drawXPos += state->ds.fullInfoHlColWidth*state->ds.uiUserScale;
      if(dat->ndat.levels[lvlInd].numTran > 0){
        drawYPos = levelStartDrawPos;
        for(uint16_t i=0; i<dat->ndat.levels[lvlInd].numTran; i++){
          float drawXPosTran = drawXPos;
          getGammaEnergyStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i),1);
          drawSelectableTextAlignedSized(rdat,&state->tss,drawXPosTran,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw transition energy label
          drawXPosTran += state->ds.fullInfoEgammaColWidth*state->ds.uiUserScale;
          getGammaIntensityStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i),1);
          drawSelectableTextAlignedSized(rdat,&state->tss,drawXPosTran,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw transition intensity label
          drawXPosTran += state->ds.fullInfoIgammaColWidth*state->ds.uiUserScale;
          if(drawMode <= LLDRAWMODE_SKIPICC){
            getGammaMultipolarityStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i));
            drawSelectableTextAlignedSized(rdat,&state->tss,drawXPosTran,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw transition multipolarity label
            drawXPosTran += state->ds.fullInfoMgammaColWidth*state->ds.uiUserScale;
          }
          if(drawMode == LLDRAWMODE_ALL){
            getGammaICCStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i),1);
            drawSelectableTextAlignedSized(rdat,&state->tss,drawXPosTran,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw transition ICC label
            drawXPosTran += state->ds.fullInfoICCColWidth*state->ds.uiUserScale;
          }
          if(dat->ndat.tran[(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i)].finalLvlOffset != 0){
            float drawXPosFL = drawXPosTran;
            uint32_t finalLvlInd = getFinalLvlInd(&dat->ndat,lvlInd,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i));
            getLvlEnergyStr(tmpStr,&dat->ndat,finalLvlInd,0);
            drawSelectableTextAlignedSized(rdat,&state->tss,drawXPosFL,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw final level energy label
            if(drawMode <= LLDRAWMODE_SKIPICC_MULT){
              drawXPosFL += (state->ds.fullInfoFinalElevelColWidth+state->ds.fullInfoFinalJpiColWidth)*state->ds.uiUserScale;
              getSpinParStr(tmpStr,&dat->ndat,finalLvlInd);
              drawSelectableTextAlignedSized(rdat,&state->tss,drawXPosFL,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384); //draw final level spin-parity label
            }
          }

          drawYPos += (NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale + txtYOffset);
        }
      }
      drawYPos = levelStartDrawPos + numLines*(NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale + txtYOffset);
    }else{
      if(drawYPos > state->ds.windowYRes){
        break;
      }
      drawYPos += (numLines*(NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale + txtYOffset));
    }
    //draw divider line
    drawLine(rdat,floorf(rect.x),ceilf(drawYPos),ceilf(rect.x+rect.w),ceilf(drawYPos),ceilf(NUCL_FULLINFOBOX_DIVIDER_LINE_THICKNESS*state->ds.uiUserScale),dividerLineCol,dividerLineCol);
    
  }

  //scroll bar
  if(rdat->ssdat.takingScreenshot != 1){
    if(state->ds.nuclFullInfoMaxScrollY > 0){
      rect.x = (float)(state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
      rect.y = (float)(state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
      rect.w = (float)(state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
      rect.h = (float)(state->ds.uiElemHeight[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
      const float sbPos = state->ds.nuclFullInfoScrollY/state->ds.nuclFullInfoMaxScrollY;
      float sbViewSize = (float)(getNumScreenLvlDispLines(&state->ds))/(float)(getNumTotalLvlDispLines(&dat->ndat,state));
      if(sbViewSize < 0.05f){sbViewSize = 0.05f;} //enforce minimum scrollbar size
      const float sbAlpha = (float)(txtAlpha/255.0f);
      //SDL_Log("x: %f, y: %f, w: %f. h: %f\n",(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
      //SDL_Log("pos: %f, view size: %f, alpha: %f\n",(double)sbPos,(double)sbViewSize,(double)sbAlpha);
      drawScrollBar(&dat->rules.themeRules,rdat,rect,getHighlightState(state,UIELEM_NUCL_FULLINFOBOX_SCROLLBAR),sbAlpha,sbPos,sbViewSize);
    }
  }

  //rect to hide over-scrolled level info
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.w = state->ds.windowXRes;
  rect.h = NUCL_FULLINFOBOX_LEVELLIST_POS_Y*state->ds.uiUserScale;
  drawFlatRect(rdat,rect,dat->rules.themeRules.bgCol);
  //rect underneath column titles
  SDL_FColor tableHeaderRectCol = {0.92f,0.92f,0.92f,txtAlpha/255.0f};
  rect.y = (NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y - 2*UI_PADDING_SIZE)*state->ds.uiUserScale + txtYOffset;
  rect.h = (NUCL_FULLINFOBOX_LEVELLIST_POS_Y - NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y + 2*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawFlatRect(rdat,rect,tableHeaderRectCol);
  //draw divider line
  drawLine(rdat,0.0f,rect.y+rect.h,state->ds.windowXRes,rect.y+rect.h,NUCL_FULLINFOBOX_DIVIDER_LINE_THICKNESS*state->ds.uiUserScale,dividerLineCol,dividerLineCol);

  //header
  char descStr[64];
  drawInfoBoxHeader(dat,state,rdat,0.0f,NUCL_FULLINFOBOX_HEADER_POS_Y*state->ds.uiUserScale,255,nuclInd);
  //proton and neutron numbers, abundance
  drawYPos = NUCL_FULLINFOBOX_NZVALS_POS_Y*state->ds.uiUserScale + txtYOffset;
  SDL_snprintf(descStr,64,"%s: %3i, %s: %3i",dat->strings[dat->locStringIDs[LOCSTR_PROTONSDESC]],dat->ndat.nuclData[nuclInd].Z,dat->strings[dat->locStringIDs[LOCSTR_NEUTRONSDESC]],dat->ndat.nuclData[nuclInd].N);
  drawSelectableTextAlignedSized(rdat,&state->tss,NUCL_FULLINFOBOX_NZVALS_POS_X*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_SMALL,txtAlpha,descStr,ALIGN_LEFT,16384);
  drawYPos += 18.0f*state->ds.uiUserScale;
  if(dat->ndat.nuclData[nuclInd].abundance.val > 0.0f){
    getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
    SDL_snprintf(descStr,64,"%s of %s on Earth",tmpStr,getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,255));
    drawSelectableTextAlignedSized(rdat,&state->tss,NUCL_FULLINFOBOX_NZVALS_POS_X*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_SMALL,txtAlpha,descStr,ALIGN_LEFT,16384);
  }else{
    drawSelectableTextAlignedSized(rdat,&state->tss,NUCL_FULLINFOBOX_NZVALS_POS_X*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_SMALL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_NOTNATURAL]],ALIGN_LEFT,16384);
  }
  drawYPos += 18.0f*state->ds.uiUserScale;
  SDL_snprintf(descStr,64,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_ELEM_TYPE]],getElementFamilyStr(dat,dat->ndat.nuclData[nuclInd].Z));
  drawSelectableTextAlignedSized(rdat,&state->tss,NUCL_FULLINFOBOX_NZVALS_POS_X*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_SMALL,txtAlpha,descStr,ALIGN_LEFT,16384);
  
  //Q-values and masses
  char massStr[48]; //need a longer string, some masses have many decimal places!
  drawXPos = state->ds.windowXRes - NUCL_FULLINFOBOX_QVAL_POS_XR*state->ds.uiUserScale;
  drawYPos = NUCL_FULLINFOBOX_QVAL_POS_Y*state->ds.uiUserScale + txtYOffset;
  if(dat->ndat.nuclData[nuclInd].massAMU.val != 0.0){
    getMassValStr(descStr,dat->ndat.nuclData[nuclInd].massAMU,1);
    SDL_snprintf(massStr,48,"%s: %s %s",dat->strings[dat->locStringIDs[LOCSTR_ATOMIC_MASS]],descStr,getValueUnitShortStr(VALUE_UNIT_AMU));
  }else{
    SDL_snprintf(massStr,48,"%s",dat->strings[dat->locStringIDs[LOCSTR_MASS_UNKNOWN]]);
  }
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,massStr,ALIGN_RIGHT,16384);
  drawYPos += 20.0f*state->ds.uiUserScale;
  if(dat->ndat.nuclData[nuclInd].sp.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].sp,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_SP]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].sp.unit));
    rect = drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
    drawXPos -= rect.w + 4*UI_PADDING_SIZE*state->ds.uiUserScale;
  }
  if(dat->ndat.nuclData[nuclInd].sn.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].sn,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_SN]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].sn.unit));
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
  }
  drawXPos = state->ds.windowXRes - NUCL_FULLINFOBOX_QVAL_POS_XR*state->ds.uiUserScale;
  drawYPos += 20.0f*state->ds.uiUserScale;
  if(dat->ndat.nuclData[nuclInd].qbeta.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].qbeta,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_QBETAMNUS]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].qbeta.unit));
    rect = drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
    drawXPos -= rect.w + 4*UI_PADDING_SIZE*state->ds.uiUserScale;
  }
  if(dat->ndat.nuclData[nuclInd].qalpha.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].qalpha,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_QALPHA]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].qalpha.unit));
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
  }

  //draw column title strings
  drawXPos = origDrawXPos;
  drawYPos = (NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y - 2.0f)*state->ds.uiUserScale + txtYOffset;
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos - 2*UI_PADDING_SIZE*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL_BOLD,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_LEVELINFO_HEADER]],ALIGN_LEFT,16384);
  drawYPos += (NUCL_INFOBOX_SMALLLINE_HEIGHT + UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_ENERGY_KEV]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoElevelColWidth*state->ds.uiUserScale;
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_JPI]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoJpiColWidth*state->ds.uiUserScale;
  if(state->ds.useLifetimes){
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_LIFETIME]],ALIGN_LEFT,16384);
  }else{
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_HALFLIFE]],ALIGN_LEFT,16384);
  }
  drawXPos += state->ds.fullInfoHlColWidth*state->ds.uiUserScale;
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_ENERGY_GAMMA]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoEgammaColWidth*state->ds.uiUserScale;
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_INTENSITY_GAMMA]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoIgammaColWidth*state->ds.uiUserScale;
  if(drawMode <= LLDRAWMODE_SKIPICC){
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_MULTIPOLARITY_GAMMA]],ALIGN_LEFT,16384);
    drawXPos += state->ds.fullInfoMgammaColWidth*state->ds.uiUserScale;
  }
  if(drawMode == LLDRAWMODE_ALL){
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_ICC_GAMMA]],ALIGN_LEFT,16384);
    drawXPos += state->ds.fullInfoICCColWidth*state->ds.uiUserScale;
  }
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_FINALLEVEL]],ALIGN_LEFT,16384);

  //draw buttons
  if(rdat->ssdat.takingScreenshot != 1){
    //back button
    drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON],state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON],state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON],getHighlightState(state,UIELEM_NUCL_FULLINFOBOX_BACKBUTTON),255,UIICON_DOWNARROWS,dat->strings[dat->locStringIDs[LOCSTR_BACKTOSUMMARY]]);
    //reaction selector button
    if(state->ds.selectedRxn == 0){
      drawDropDownTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] - txtYOffset),state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON],getHighlightState(state,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON),txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_ALLREACTIONS]]);
    }else{
      char rxnStr[32];
      getRxnStr(rxnStr,&dat->ndat,dat->ndat.nuclData[state->chartSelectedNucl].firstRxn + (uint32_t)(state->ds.selectedRxn-1));
      drawDropDownTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] - txtYOffset),state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON],getHighlightState(state,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON),txtAlpha,rxnStr);
    }
  }
  
}

void drawNuclInfoBox(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t nuclInd){
  
  uint16_t yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_HIDE)){
    yOffset = (uint16_t)(300.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_SHOW)){
    yOffset = (uint16_t)(300.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW]/(UI_ANIM_LENGTH)));
  }
  Uint8 alpha = 255;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
    alpha = (uint8_t)(255.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
    alpha = (uint8_t)(255.0f*juice_smoothStop2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH));
  }
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_MORPH)){
    float morphFrac = juice_smoothStop3(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_MORPH]/SHORT_UI_ANIM_LENGTH);
    state->ds.infoBoxCurrentDispWidth = state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]*morphFrac + state->ds.infoBoxPrevDispWidth*(1.0f - morphFrac);
    state->ds.infoBoxCurrentDispHeight = state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX]*morphFrac + state->ds.infoBoxPrevDispHeight*(1.0f - morphFrac);
    state->ds.infoBoxCurrentX = state->ds.infoBoxPrevX - (state->ds.infoBoxCurrentDispWidth-state->ds.infoBoxPrevDispWidth)/2.0f;
    state->ds.infoBoxCurrentY = state->ds.infoBoxPrevY - (state->ds.infoBoxCurrentDispHeight-state->ds.infoBoxPrevDispHeight);
    if(alpha == 255){
      alpha = (uint8_t)(255.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_MORPH]/SHORT_UI_ANIM_LENGTH));
    }
  }else{
    state->ds.infoBoxCurrentX = state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX];
    state->ds.infoBoxCurrentY = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX];
    state->ds.infoBoxCurrentDispWidth = state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX];
    state->ds.infoBoxCurrentDispHeight = state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX];
  }

  //draw panel background
  SDL_FRect infoBoxPanelRect;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
    //expand from normal size to full screen
    float animFrac = juice_smoothStop3(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH);
    infoBoxPanelRect.x = state->ds.infoBoxCurrentX*(1.0f - animFrac);
    infoBoxPanelRect.y = (state->ds.infoBoxCurrentY + yOffset)*(1.0f - animFrac);
    infoBoxPanelRect.w = state->ds.infoBoxCurrentDispWidth + animFrac*(state->ds.windowXRes - state->ds.infoBoxCurrentDispWidth);
    infoBoxPanelRect.h = state->ds.infoBoxCurrentDispHeight + animFrac*(state->ds.windowYRes - state->ds.infoBoxCurrentDispHeight);
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
    //contract from full screen to normal size
    float animFrac = juice_smoothStop3(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH);
    infoBoxPanelRect.x = 0.0f + animFrac*(state->ds.infoBoxCurrentX);
    infoBoxPanelRect.y = 0.0f + animFrac*(state->ds.infoBoxCurrentY + yOffset);
    infoBoxPanelRect.w = state->ds.windowXRes + animFrac*(state->ds.infoBoxCurrentDispWidth - state->ds.windowXRes);
    infoBoxPanelRect.h = state->ds.windowYRes + animFrac*(state->ds.infoBoxCurrentDispHeight - state->ds.windowYRes);
  }else{
    //normal sized info box
    infoBoxPanelRect.x = state->ds.infoBoxCurrentX;
    infoBoxPanelRect.y = state->ds.infoBoxCurrentY + yOffset;
    infoBoxPanelRect.w = state->ds.infoBoxCurrentDispWidth;
    infoBoxPanelRect.h = state->ds.infoBoxCurrentDispHeight;
  }
  drawPanelBG(&dat->rules.themeRules,rdat,infoBoxPanelRect,1.0f);
  
  removeSelectableStringsInRect(&state->tss,infoBoxPanelRect); //get rid of selectable strings behind the panel

  //draw column title strings
  char tmpStr[32];
  float drawXPos = (float)(infoBoxPanelRect.x + (state->ds.infoBoxEColOffset)*state->ds.uiUserScale);
  float drawYPos = (float)(infoBoxPanelRect.y + (state->ds.infoBoxEColOffset + 32.0f)*state->ds.uiUserScale);
  if(dat->ndat.nuclData[nuclInd].abundance.val > 0.0f){
    char abundanceStr[64];
    getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
    SDL_snprintf(abundanceStr,64,"...is %s of %s on Earth.",tmpStr,getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,255));
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_SMALL,alpha,abundanceStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
		drawYPos += NUCL_INFOBOX_ABUNDANCE_LINE_HEIGHT*state->ds.uiUserScale;
	}else{
    drawYPos += 6.0f*state->ds.uiUserScale;
  }
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos + 2.0f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL_BOLD,alpha,dat->strings[dat->locStringIDs[LOCSTR_GM_STATE]],ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  drawYPos += NUCL_INFOBOX_BIGLINE_HEIGHT*state->ds.uiUserScale;
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_ENERGY_KEV]],ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxJpiColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_JPI]],ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  if(state->ds.useLifetimes){
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_LIFETIME]],ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  }else{
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_HALFLIFE]],ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  }
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_DECAYMODE]],ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));

  //draw divider line
  drawYPos += 0.9f*NUCL_INFOBOX_BIGLINE_HEIGHT*state->ds.uiUserScale;
  drawLine(rdat,drawXPos,drawYPos,drawXPos + infoBoxPanelRect.w - 2.0f*state->ds.infoBoxEColOffset*state->ds.uiUserScale,drawYPos,NUCL_FULLINFOBOX_DIVIDER_LINE_THICKNESS*state->ds.uiUserScale,lightGrayCol,lightGrayCol);

  //ground state
  drawYPos += 0.2f*NUCL_INFOBOX_BIGLINE_HEIGHT*state->ds.uiUserScale;
  uint32_t lvlInd = dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel;
  getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,0);
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  getSpinParStr(tmpStr,&dat->ndat,lvlInd);
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxJpiColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  getHalfLifeStr(tmpStr,dat,lvlInd,1,1,state->ds.useLifetimes);
  drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
  if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,"N/A",ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth)); //draw no decay mode label
    drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
  }else{
    if(dat->ndat.levels[lvlInd].numDecModes > 0){
      for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
        getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
        //SDL_Log("%s\n",tmpStr);
        drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth)); //draw decay mode label
        drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
      }
    }else{
      drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth)); //draw decay mode label
      drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
    }
    
  }

  //longest isomer
  //SDL_Log("Isomer index: %u\n",dat->ndat.nuclData[nuclInd].longestIsomerLevel);
  lvlInd = dat->ndat.nuclData[nuclInd].longestIsomerLevel;
  if((lvlInd != MAXNUMLVLS)&&(lvlInd != (dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel))){
    drawYPos += ((NUCL_INFOBOX_BIGLINE_HEIGHT - NUCL_INFOBOX_SMALLLINE_HEIGHT)*state->ds.uiUserScale);
    getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,1);
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
    getSpinParStr(tmpStr,&dat->ndat,lvlInd);
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxJpiColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
    getHalfLifeStr(tmpStr,dat,lvlInd,1,1,state->ds.useLifetimes);
    drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth));
    if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
      drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,"N/A",ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth)); //draw no decay mode label
    }else{
      if(dat->ndat.levels[lvlInd].numDecModes > 0){
        for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
          getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
          //SDL_Log("%s\n",tmpStr);
          drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth)); //draw decay mode label
          drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
        }
      }else if(dat->ndat.levels[lvlInd].numTran >0){
        SDL_strlcpy(tmpStr,"IT > 0%",32);
        drawSelectableTextAlignedSized(rdat,&state->tss,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,(Uint16)(state->ds.infoBoxCurrentDispWidth)); //draw decay mode label
        drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
      }
    }
  }

  //header
  drawInfoBoxHeader(dat,state,rdat,infoBoxPanelRect.x,infoBoxPanelRect.y,255,nuclInd);

  //draw buttons
  if(rdat->ssdat.takingScreenshot != 1){
    
    //all level info button
    updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
    if((state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND))||(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT))){ 
      drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],getHighlightState(state,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON),255,UIICON_UPARROWS,dat->strings[dat->locStringIDs[LOCSTR_ALLLEVELS]]);
    }else{
      SDL_FRect allLvlButtonPos = getInfoBoxAllLvlButtonPos(state,state->ds.infoBoxCurrentX,state->ds.infoBoxCurrentY,state->ds.infoBoxCurrentDispWidth);
      drawXPos = allLvlButtonPos.x + 0.5f*(infoBoxPanelRect.w - state->ds.infoBoxCurrentDispWidth);
      drawYPos = allLvlButtonPos.y + yOffset;
      drawIconAndTextButton(&dat->rules.themeRules,rdat,(int16_t)drawXPos,(int16_t)drawYPos,(int16_t)allLvlButtonPos.w,getHighlightState(state,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON),255,UIICON_UPARROWS,dat->strings[dat->locStringIDs[LOCSTR_ALLLEVELS]]);
    }
    
    //close button/icon
    alpha = 255;
    if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
      //expand from normal size to full screen
      alpha = (uint8_t)(255.0f*(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH));
    }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
      alpha = (uint8_t)(255.0f*(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH));
    }
    SDL_FRect closeButtonPos = getInfoBoxCloseButtonPos(state,state->ds.infoBoxCurrentX,state->ds.infoBoxCurrentY,state->ds.infoBoxCurrentDispWidth);
    drawXPos = closeButtonPos.x + 0.5f*(infoBoxPanelRect.w - state->ds.infoBoxCurrentDispWidth);
    drawYPos = closeButtonPos.y + infoBoxPanelRect.y - state->ds.infoBoxCurrentY;
    drawIcon(&dat->rules.themeRules,rdat,(int16_t)drawXPos,(int16_t)drawYPos,(int16_t)closeButtonPos.w,getHighlightState(state,UIELEM_NUCL_INFOBOX_CLOSEBUTTON),(float)(alpha/255.0f),UIICON_CLOSE);
  }
  
  //SDL_Log("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE],alpha);
}

void drawAboutBox(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0.0f;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_HIDE)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_SHOW)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    yOffset = (100.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/(UI_ANIM_LENGTH)));
  }else{
    drawScreenDimmer(&state->ds,rdat,DIMMER_OPACITY);
  }
  const uint8_t alpha8 = (uint8_t)SDL_floorf(alpha*255.0f);
  
  SDL_FRect aboutBoxPanelRect;
  aboutBoxPanelRect.x = state->ds.uiElemPosX[UIELEM_ABOUT_BOX];
  aboutBoxPanelRect.y = state->ds.uiElemPosY[UIELEM_ABOUT_BOX] + yOffset;
  aboutBoxPanelRect.w = state->ds.uiElemWidth[UIELEM_ABOUT_BOX];
  aboutBoxPanelRect.h = state->ds.uiElemHeight[UIELEM_ABOUT_BOX];
  drawPanelBG(&dat->rules.themeRules,rdat,aboutBoxPanelRect,alpha);

  drawIcon(&dat->rules.themeRules,rdat,(int16_t)(aboutBoxPanelRect.x + UI_TILE_SIZE*state->ds.uiUserScale),(int16_t)(aboutBoxPanelRect.y+(ABOUT_BOX_HEADERTXT_Y+UI_PADDING_SIZE)*state->ds.uiUserScale),(int16_t)(UI_TILE_SIZE*state->ds.uiUserScale),HIGHLIGHT_NORMAL,alpha,UIICON_APPICON);
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(2*UI_TILE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale,aboutBoxPanelRect.y+ABOUT_BOX_HEADERTXT_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_LARGE,alpha8,dat->rules.appName,ALIGN_LEFT,(Uint16)(aboutBoxPanelRect.w - 2*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(2*UI_TILE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale,aboutBoxPanelRect.y+ABOUT_BOX_VERSION_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_SMALL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_VERSION]],ALIGN_LEFT,16384);
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR1_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL_BOLD,alpha8,dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_1]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 16*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR2_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_SMALL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_2]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 16*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR3_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_SMALL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_3]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 16*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR4_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_SMALL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_4]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 16*UI_PADDING_SIZE*state->ds.uiUserScale));
  //close button
  drawIcon(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_ABOUT_BOX_CLOSEBUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_ABOUT_BOX_CLOSEBUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_ABOUT_BOX_CLOSEBUTTON],getHighlightState(state,UIELEM_ABOUT_BOX_CLOSEBUTTON),alpha,UIICON_CLOSE);
  //SDL_Log("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE],alpha);
}

uint16_t getStrIndForUIScale(const app_data *restrict dat, const drawing_state *restrict ds){
  switch(ds->interfaceSizeInd){
    case UISCALE_SMALL:
      return dat->locStringIDs[LOCSTR_SMALL];
      break;
    case UISCALE_LARGE:
      return dat->locStringIDs[LOCSTR_LARGE];
      break;
    case UISCALE_HUGE:
      return dat->locStringIDs[LOCSTR_HUGE];
      break;
    case UISCALE_NORMAL:
    default:
      return dat->locStringIDs[LOCSTR_DEFAULT];
      break;
  }
}

void drawPrefsDialog(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_HIDE)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_SHOW)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    yOffset = (100.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/(UI_ANIM_LENGTH)));
  }else{
    drawScreenDimmer(&state->ds,rdat,DIMMER_OPACITY);
  }
  
  SDL_FRect prefsDialogPanelRect;
  prefsDialogPanelRect.x = state->ds.uiElemPosX[UIELEM_PREFS_DIALOG];
  prefsDialogPanelRect.y = state->ds.uiElemPosY[UIELEM_PREFS_DIALOG] + yOffset;
  prefsDialogPanelRect.w = state->ds.uiElemWidth[UIELEM_PREFS_DIALOG];
  prefsDialogPanelRect.h = state->ds.uiElemHeight[UIELEM_PREFS_DIALOG];
  drawPanelBG(&dat->rules.themeRules,rdat,prefsDialogPanelRect,alpha);

  uint8_t alpha8 = (uint8_t)SDL_floorf(alpha*255.0f);
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+PREFS_DIALOG_HEADERTXT_X*state->ds.uiUserScale,prefsDialogPanelRect.y+PREFS_DIALOG_HEADERTXT_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_LARGE,alpha8,dat->strings[dat->locStringIDs[LOCSTR_MENUITEM_PREFS]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_TILE_SIZE+2*UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+6.0f+(PREFS_DIALOG_PREF_Y_SPACING)+(2*UI_PADDING_SIZE))*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_SHELLCLOSURE]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_TILE_SIZE+2*UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+6.0f+(2*PREFS_DIALOG_PREF_Y_SPACING)+(2*UI_PADDING_SIZE))*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_LIFETIME]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_TILE_SIZE+2*UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+6.0f+(3*PREFS_DIALOG_PREF_Y_SPACING)+(2*UI_PADDING_SIZE))*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_UIANIM]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+2.0f)*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_UISCALE]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawCheckbox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX],(int16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX],getHighlightState(state,UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX),alpha,state->ds.drawShellClosures);
  drawCheckbox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX],(int16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX],getHighlightState(state,UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX),alpha,state->ds.useLifetimes);
  drawCheckbox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX],(int16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX],getHighlightState(state,UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX),alpha,state->ds.useUIAnimations);
  drawDropDownTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN],(int16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN],getHighlightState(state,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN),alpha8,dat->strings[getStrIndForUIScale(dat,&state->ds)]);
  //close button
  drawIcon(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_CLOSEBUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_CLOSEBUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON],getHighlightState(state,UIELEM_PREFS_DIALOG_CLOSEBUTTON),alpha,UIICON_CLOSE);
  
  //SDL_Log("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE],alpha);
}

void drawUIScaleMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_UISCALE_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_UISCALE_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PREFS_UISCALE_MENU];
  const int16_t arrowX = (int16_t)(state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] + 0.39f*state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN]);
  drawMenuBGWithArrow(&dat->rules.themeRules,rdat,drawRect,arrowX,alpha);
  
  //draw menu item highlight
  if(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f){
    for(uint8_t i=1;i<=UISCALE_ENUM_LENGTH;i++){
      drawRect.x = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU-i];
      drawRect.y = (state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU-i] + yOffset);
      drawRect.w = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU-i];
      drawRect.h = state->ds.uiElemHeight[UIELEM_PREFS_UISCALE_MENU-i];
      switch(getHighlightState(state,UIELEM_PREFS_UISCALE_MENU-i)){
        case HIGHLIGHT_SELECTED:
          drawFlatRect(rdat,drawRect,dat->rules.themeRules.modSelectedCol);
          break;
        case HIGHLIGHT_MOUSEOVER:
          drawFlatRect(rdat,drawRect,dat->rules.themeRules.modMouseOverCol);
          break;
        case HIGHLIGHT_NORMAL:
        default:
          break;
      }
    }
  }

  //draw menu item text
  drawRect.x = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PREFS_UISCALE_MENU];
  const Uint8 txtAlpha = (Uint8)(alpha*255.0f);
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 0.4f*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_SMALL]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 1.4f*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_DEFAULT]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 2.4f*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_LARGE]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 3.4f*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_HUGE]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));

}

void drawRxnMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_RXN_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_RXN_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  const uint8_t alpha8 = (uint8_t)SDL_floorf(alpha*255.0f);
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_RXN_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_RXN_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_RXN_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_RXN_MENU];
  const int16_t arrowX = (int16_t)(state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] + 0.42f*state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON]);
  drawMenuBGWithArrow(&dat->rules.themeRules,rdat,drawRect,arrowX,alpha);

  const uint8_t numRxnPerCol = getRxnMenuNumRxnsPerColumn(dat,state);

  //draw menu item highlight
  SDL_FColor highlightCol;
  //SDL_Log("numRxns: %u\n",dat->ndat.nuclData[state->chartSelectedNucl].numRxns);
  //SDL_Log("Mouse-over reaction: %u, selected reaction: %u.\n",state->ds.mouseOverRxn,state->ds.selectedRxn);
  for(uint8_t i=0;i<(dat->ndat.nuclData[state->chartSelectedNucl].numRxns+1);i++){
    if(state->lastInputType == INPUT_TYPE_MOUSE){
      if((state->ds.selectedRxn == i)||(state->ds.mouseHoldRxn == i)){
        drawRect = getRxnMenuButtonRect(&state->ds,numRxnPerCol,i);
        drawRect.y += yOffset;
        if(state->ds.mouseOverRxn == i){
          highlightCol = dat->rules.themeRules.modSelectedAndMouseOverCol;
        }else{
          highlightCol = dat->rules.themeRules.modSelectedCol;
        }
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
      }else if(state->ds.mouseOverRxn == i){
        drawRect = getRxnMenuButtonRect(&state->ds,numRxnPerCol,i);
        drawRect.y += yOffset;
        highlightCol = dat->rules.themeRules.modMouseOverCol;
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
      }
    }else{
      if(state->ds.mouseHoldRxn == i){
        drawRect = getRxnMenuButtonRect(&state->ds,numRxnPerCol,i);
        drawRect.y += yOffset;
        highlightCol = dat->rules.themeRules.modSelectedCol;
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
      }else if(state->ds.mouseOverRxn == i){
        drawRect = getRxnMenuButtonRect(&state->ds,numRxnPerCol,i);
        drawRect.y += yOffset;
        highlightCol = dat->rules.themeRules.modMouseOverCol;
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
      }
    }
    
    if((i>state->ds.mouseOverRxn)&&(i>state->ds.selectedRxn)){
      break;
    }
  }

  //draw menu item text
  drawRect.x = state->ds.uiElemPosX[UIELEM_RXN_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawRect.w = state->ds.uiElemWidth[UIELEM_RXN_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_RXN_MENU];
  drawTextAlignedSized(rdat,drawRect.x,((float)state->ds.uiElemPosY[UIELEM_RXN_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale + 0.4f*RXN_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset),blackCol8Bit,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_ALLREACTIONS]],ALIGN_LEFT,(Uint16)(RXN_MENU_COLUMN_WIDTH*state->ds.uiUserScale));
  char rxnStr[32];
  for(uint8_t i=0; i<dat->ndat.nuclData[state->chartSelectedNucl].numRxns; i++){
    getRxnStr(rxnStr,&dat->ndat,dat->ndat.nuclData[state->chartSelectedNucl].firstRxn + (uint32_t)i);
    if(UTF8Strlen(rxnStr) > RXN_MENU_ITEM_MAXCHARS){
      //truncate string so that it fits on the button
      //but don't truncate on a non-ASCII character
      const uint8_t strTruncInd=(uint8_t)(RXN_MENU_ITEM_MAXCHARS-3);
      uint8_t lastAsciiInd=0;
      for(uint8_t j=0;j<=strTruncInd;j++){
        if(charIsAscii(rxnStr[j])){
          lastAsciiInd=j;
        }
      }
      rxnStr[lastAsciiInd] = '.';
      rxnStr[lastAsciiInd+1] = '.';
      rxnStr[lastAsciiInd+2] = '.';
      rxnStr[lastAsciiInd+3] = '\0'; //terminate string
    }
    drawRect.x = state->ds.uiElemPosX[UIELEM_RXN_MENU] + (float)(PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE + RXN_MENU_COLUMN_WIDTH*((i+1)/numRxnPerCol))*state->ds.uiUserScale;
    drawRect.y = (float)state->ds.uiElemPosY[UIELEM_RXN_MENU] + (PANEL_EDGE_SIZE + ((float)((i+1)%numRxnPerCol) + 0.4f)*RXN_MENU_ITEM_SPACING)*state->ds.uiUserScale + yOffset;
    drawTextAlignedSized(rdat,drawRect.x,drawRect.y,blackCol8Bit,FONTSIZE_NORMAL,alpha8,rxnStr,ALIGN_LEFT,(Uint16)(RXN_MENU_COLUMN_WIDTH*state->ds.uiUserScale));
  }

}

void drawSearchMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_SEARCH_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_SEARCH_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  const uint8_t alpha8 = (uint8_t)SDL_floorf(alpha*255.0f);
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_SEARCH_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_SEARCH_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_SEARCH_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_SEARCH_MENU];
  const int16_t arrowX = (int16_t)(drawRect.x + 0.44f*drawRect.w);
  drawMenuBGWithArrow(&dat->rules.themeRules,rdat,drawRect,arrowX,alpha);

  //draw entry box
  if(state->ds.searchEntryDispNumChars == 65535U){
    drawIconAndTextEntryBox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_SEARCH_ENTRYBOX],(int16_t)(state->ds.uiElemPosY[UIELEM_SEARCH_ENTRYBOX]+yOffset),state->ds.uiElemWidth[UIELEM_SEARCH_ENTRYBOX],getHighlightState(state,UIELEM_SEARCH_ENTRYBOX),HIGHLIGHT_INACTIVE,alpha8,UIICON_SEARCHGRAY,dat->strings[dat->locStringIDs[LOCSTR_SEARCH_PLACEHOLDER]],0,256,-1);
  }else{
    drawIconAndTextEntryBox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_SEARCH_ENTRYBOX],(int16_t)(state->ds.uiElemPosY[UIELEM_SEARCH_ENTRYBOX]+yOffset),state->ds.uiElemWidth[UIELEM_SEARCH_ENTRYBOX],getHighlightState(state,UIELEM_SEARCH_ENTRYBOX),HIGHLIGHT_NORMAL,alpha8,UIICON_SEARCH,state->ss.searchString,state->ds.searchEntryDispStartChar,state->ds.searchEntryDispNumChars,state->searchCursorPos);
  }

  //draw results
  for(uint8_t i=0; i<state->ss.numResults; i++){
    char tmpStr[64];
    char eStr[32];
    uint16_t Z,N;
    float numWidth;
    if(i<MAX_DISP_SEARCH_RESULTS){
      drawRect.x = state->ds.uiElemPosX[UIELEM_SEARCH_RESULT+i];
      drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_SEARCH_RESULT+i] + yOffset);
      drawRect.w = state->ds.uiElemWidth[UIELEM_SEARCH_RESULT+i];
      drawRect.h = state->ds.uiElemHeight[UIELEM_SEARCH_RESULT+i];
      drawButtonBG(&dat->rules.themeRules,rdat,drawRect,getHighlightState(state,UIELEM_SEARCH_RESULT+i),alpha);
      //draw relevance value
      SDL_snprintf(eStr,32,"%0.3f",(double)state->ss.results[i].relevance);
      drawTextAlignedSized(rdat,drawRect.x+drawRect.w-10.0f*state->ds.uiUserScale,drawRect.y+8.0f*state->ds.uiUserScale,grayCol8Bit,FONTSIZE_SMALL,alpha8,eStr,ALIGN_RIGHT,16384);
      //draw result info
      switch(state->ss.results[i].resultType){
        case SEARCHAGENT_NUCLIDE:
          Z = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].Z;
          N = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].N;
          SDL_snprintf(tmpStr,64,"%u",N+Z);
          numWidth = drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+12.0f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_SMALL,alpha8,tmpStr,ALIGN_LEFT,16384).w; //draw number label
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale+numWidth,drawRect.y+(20.0f*state->ds.uiUserScale),blackCol8Bit,FONTSIZE_LARGE,alpha8,getElemStr((uint8_t)Z),ALIGN_LEFT,16384); //draw element label
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+(SEARCH_MENU_RESULT_HEIGHT-32.0f)*state->ds.uiUserScale,grayCol8Bit,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_SEARCHRES_NUCLIDE]],ALIGN_LEFT,16384);
          break;
        case SEARCHAGENT_EGAMMA:
          Z = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].Z;
          N = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].N;
          SDL_snprintf(tmpStr,64,"%u",N+Z);
          numWidth = drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+12.0f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_SMALL,alpha8,tmpStr,ALIGN_LEFT,16384).w; //draw number label
          getGammaEnergyStr(eStr,&dat->ndat,state->ss.results[i].resultVal[1],1);
          SDL_snprintf(tmpStr,64,"%s - %s keV",getElemStr((uint8_t)Z),eStr);
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale+numWidth,drawRect.y+(20.0f*state->ds.uiUserScale),blackCol8Bit,FONTSIZE_LARGE,alpha8,tmpStr,ALIGN_LEFT,16384); //draw element and gamma label
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+(SEARCH_MENU_RESULT_HEIGHT-32.0f)*state->ds.uiUserScale,grayCol8Bit,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_SEARCHRES_EGAMMA]],ALIGN_LEFT,16384);
          break;
        case SEARCHAGENT_ELEVEL:
          Z = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].Z;
          N = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].N;
          SDL_snprintf(tmpStr,64,"%u",N+Z);
          numWidth = drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+12.0f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_SMALL,alpha8,tmpStr,ALIGN_LEFT,16384).w; //draw number label
          getLvlEnergyStr(eStr,&dat->ndat,state->ss.results[i].resultVal[1],1);
          SDL_snprintf(tmpStr,64,"%s - %s keV",getElemStr((uint8_t)Z),eStr);
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale+numWidth,drawRect.y+(20.0f*state->ds.uiUserScale),blackCol8Bit,FONTSIZE_LARGE,alpha8,tmpStr,ALIGN_LEFT,16384); //draw element and level label
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+(SEARCH_MENU_RESULT_HEIGHT-32.0f)*state->ds.uiUserScale,grayCol8Bit,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_SEARCHRES_ELEVEL]],ALIGN_LEFT,16384);
          break;
        case SEARCHAGENT_GAMMACASCADE:
          Z = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].Z;
          N = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].N;
          SDL_snprintf(tmpStr,64,"%u",N+Z);
          numWidth = drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+12.0f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_SMALL,alpha8,tmpStr,ALIGN_LEFT,16384).w; //draw number label
          int32_t length = 0;
          uint8_t numCascadeGammas = 0;
          for(uint8_t j=1; j<SEARCH_RESULT_DATASIZE; j++){
            if(state->ss.results[i].resultVal[j] == UNUSED_SEARCH_RESULT){
              break;
            }
            numCascadeGammas++;
          }
          if(numCascadeGammas > 3){
            numCascadeGammas = 3; //truncate the string so it fits in the box
          }
          length += SDL_snprintf(tmpStr+length,64,"%s - ",getElemStr((uint8_t)Z));
          for(uint8_t j=1; j<numCascadeGammas; j++){
            length += SDL_snprintf(tmpStr+length,(uint64_t)(64-length),"%0.0f, ",(double)(dat->ndat.tran[state->ss.results[i].resultVal[j]].energy.val));
          }
          length += SDL_snprintf(tmpStr+length,(uint64_t)(64-length),"%0.0f keV",(double)(dat->ndat.tran[state->ss.results[i].resultVal[numCascadeGammas]].energy.val));
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale+numWidth,drawRect.y+(20.0f*state->ds.uiUserScale),blackCol8Bit,FONTSIZE_LARGE,alpha8,tmpStr,ALIGN_LEFT,16384); //draw element and cascade label
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+(SEARCH_MENU_RESULT_HEIGHT-32.0f)*state->ds.uiUserScale,grayCol8Bit,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_SEARCHRES_GAMMACASCADE]],ALIGN_LEFT,16384);
          break;
        case SEARCHAGENT_HALFLIFE:
          Z = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].Z;
          N = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal[0]].N;
          SDL_snprintf(tmpStr,64,"%u",N+Z);
          numWidth = drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+12.0f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_SMALL,alpha8,tmpStr,ALIGN_LEFT,16384).w; //draw number label
          getHalfLifeStr(eStr,dat,state->ss.results[i].resultVal[1],1,0,state->ds.useLifetimes);
          if(state->ss.results[i].resultVal[1] != (dat->ndat.nuclData[state->ss.results[i].resultVal[0]].firstLevel + dat->ndat.nuclData[state->ss.results[i].resultVal[0]].gsLevel)){
            SDL_snprintf(tmpStr,64,"%s* - %s",getElemStr((uint8_t)Z),eStr);
          }else{
            SDL_snprintf(tmpStr,64,"%s - %s",getElemStr((uint8_t)Z),eStr);
          }
          drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale+numWidth,drawRect.y+(20.0f*state->ds.uiUserScale),blackCol8Bit,FONTSIZE_LARGE,alpha8,tmpStr,ALIGN_LEFT,16384); //draw element and half-life label
          if(state->ds.useLifetimes){
            drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+(SEARCH_MENU_RESULT_HEIGHT-32.0f)*state->ds.uiUserScale,grayCol8Bit,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_SEARCHRES_LIFETIME]],ALIGN_LEFT,16384);
          }else{
            drawTextAlignedSized(rdat,drawRect.x+12.0f*state->ds.uiUserScale,drawRect.y+(SEARCH_MENU_RESULT_HEIGHT-32.0f)*state->ds.uiUserScale,grayCol8Bit,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_SEARCHRES_HALFLIFE]],ALIGN_LEFT,16384);
          }
          break;
        default:
          continue;
      }
    }
  }

}

void drawChartViewMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_CHARTVIEW_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_CHARTVIEW_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_CHARTVIEW_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_CHARTVIEW_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_CHARTVIEW_MENU];
  const int16_t arrowX = (int16_t)(drawRect.x + 0.62f*drawRect.w);
  drawMenuBGWithArrow(&dat->rules.themeRules,rdat,drawRect,arrowX,alpha);
  
  //draw menu item highlight
  SDL_FColor highlightCol;
  if(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f){
    for(uint8_t i=1;i<=CHARTVIEW_ENUM_LENGTH;i++){
      drawRect.x = state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU-i];
      drawRect.y = (state->ds.uiElemPosY[UIELEM_CHARTVIEW_MENU-i] + yOffset);
      drawRect.w = state->ds.uiElemWidth[UIELEM_CHARTVIEW_MENU-i];
      drawRect.h = state->ds.uiElemHeight[UIELEM_CHARTVIEW_MENU-i];
      switch(getHighlightState(state,UIELEM_CHARTVIEW_MENU-i)){
        case HIGHLIGHT_SELECTED:
          highlightCol = dat->rules.themeRules.modSelectedCol;
          highlightCol.a = alpha;
          drawFlatRect(rdat,drawRect,highlightCol);
          break;
        case HIGHLIGHT_MOUSEOVER:
          highlightCol = dat->rules.themeRules.modMouseOverCol;
          highlightCol.a = alpha;
          drawFlatRect(rdat,drawRect,highlightCol);
          break;
        case HIGHLIGHT_NORMAL:
        default:
          break;
      }
    }
  }

  //draw menu item text
  uint8_t numViewsPerCol = (uint8_t)SDL_ceilf((CHARTVIEW_ENUM_LENGTH)/(1.0f*CHARTVIEW_MENU_COLUMNS));
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_CHARTVIEW_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_CHARTVIEW_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_CHARTVIEW_MENU];
  const Uint8 txtAlpha = (Uint8)(alpha*255.0f);
  drawTextAlignedSized(rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU] + (PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*state->ds.uiUserScale,drawRect.y + 0.4f*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL_BOLD,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_MENUTITLE]],ALIGN_LEFT,(Uint16)(drawRect.w - 8*UI_PADDING_SIZE*state->ds.uiUserScale));
  for(uint8_t i=0;i<CHARTVIEW_ENUM_LENGTH;i++){
    drawRect.x = state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU] + ((UI_TILE_SIZE + PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE) + (float)(i/numViewsPerCol)*(CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE))*state->ds.uiUserScale;
    if(state->chartView == i){
      //draw checkmark icon
      drawIcon(&dat->rules.themeRules,rdat,(int16_t)(state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU] + ((PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE) + (float)(i/numViewsPerCol)*(CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE))*state->ds.uiUserScale),(int16_t)(drawRect.y + (1.25f + 1.0f*(i%numViewsPerCol))*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset),UI_TILE_SIZE,HIGHLIGHT_NORMAL,alpha,UIICON_CHECKBOX_CHECK);
    }
    if((i==0)&&(state->ds.useLifetimes)){
      drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (1.4f + 1.0f*(i%numViewsPerCol))*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_LIFETIME]],ALIGN_LEFT,(Uint16)(drawRect.w - 8*UI_PADDING_SIZE*state->ds.uiUserScale));
    }else{
      drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (1.4f + 1.0f*(i%numViewsPerCol))*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_HALFLIFE+i]],ALIGN_LEFT,(Uint16)(drawRect.w - 8*UI_PADDING_SIZE*state->ds.uiUserScale));
    }
  }

}

void drawPrimaryMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_PRIMARY_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_PRIMARY_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PRIMARY_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU];
  const int16_t arrowX = (int16_t)(drawRect.x + 0.82f*drawRect.w);
  drawMenuBGWithArrow(&dat->rules.themeRules,rdat,drawRect,arrowX,alpha);
  
  //draw menu item highlight
  SDL_FColor highlightCol;
  for(uint8_t i=1;i<=PRIMARY_MENU_NUM_UIELEMENTS;i++){
    drawRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU-i];
    drawRect.y = (state->ds.uiElemPosY[UIELEM_PRIMARY_MENU-i] + yOffset);
    drawRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU-i];
    drawRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU-i];
    switch(getHighlightState(state,UIELEM_PRIMARY_MENU-i)){
      case HIGHLIGHT_SELECTED:
        highlightCol = dat->rules.themeRules.modSelectedCol;
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
        break;
      case HIGHLIGHT_MOUSEOVER:
        highlightCol = dat->rules.themeRules.modMouseOverCol;
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
        break;
      case HIGHLIGHT_NORMAL:
      default:
        break;
    }
  }

  //draw menu item text
  drawRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PRIMARY_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU];
  const Uint8 txtAlpha = (Uint8)(alpha*255.0f);
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 0.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_MENUITEM_PREFS]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 1.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_MENUITEM_SCREENSHOT]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 2.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_MENUITEM_ABOUT]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));

}

void drawTextSelHighlight(const app_state *restrict state, resource_data *restrict rdat){

  if(state->tss.selectedStr >= state->tss.numSelStrs){
    //no string selected
    return;
  }

  SDL_FRect rawSelRect = getTextSelRect(&state->tss,rdat);
  drawFlatRect(rdat,rawSelRect,txtSelCol);

}

void drawContextMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_CONTEXT_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CONTEXT_MENU_HIDE]/(UI_ANIM_LENGTH)));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_CONTEXT_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_CONTEXT_MENU_SHOW]/UI_ANIM_LENGTH));
  }
  Uint8 txtAlpha = (Uint8)(alpha*255.0f);
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_CONTEXT_MENU];
  drawRect.y = state->ds.uiElemPosY[UIELEM_CONTEXT_MENU];
  drawRect.w = state->ds.uiElemWidth[UIELEM_CONTEXT_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_CONTEXT_MENU];
  drawPanelBG(&dat->rules.themeRules,rdat,drawRect,alpha);

  //draw header text
  if(state->cms.useHeaderText){
    drawRect.x = state->ds.uiElemPosX[UIELEM_CONTEXT_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
    drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_CONTEXT_MENU] + (2*PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale);
    drawTextAlignedSized(rdat,drawRect.x,drawRect.y,blackCol8Bit,FONTSIZE_LARGE,txtAlpha,state->cms.headerText,ALIGN_LEFT,16384);
  }
  
  //draw menu item highlight
  SDL_FColor highlightCol;
  for(uint8_t i=0;i<state->cms.numContextMenuItems;i++){
    drawRect = getContextMenuButtonRect(state,i);
    if(i==state->cms.mouseOverContextItem){
      highlightCol = dat->rules.themeRules.modMouseOverCol;
      highlightCol.a = alpha;
      drawFlatRect(rdat,drawRect,highlightCol);
    }else if(i==state->cms.clickedContextItem){
      highlightCol = dat->rules.themeRules.modSelectedCol;
      highlightCol.a = alpha;
      drawFlatRect(rdat,drawRect,highlightCol);
    }
  }

  //draw menu item text
  char tmpStr[32];
  drawRect.x = state->ds.uiElemPosX[UIELEM_CONTEXT_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_CONTEXT_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale);
  if(state->cms.useHeaderText){
    drawRect.y += (float)(CONTEXT_MENU_HEADER_HEIGHT*state->ds.uiUserScale);
  }
  drawRect.w = state->ds.uiElemWidth[UIELEM_CONTEXT_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_CONTEXT_MENU];
  for(uint8_t i=0;i<state->cms.numContextMenuItems;i++){
    switch(state->cms.contextMenuItems[i]){
      case CONTEXTITEM_COPY:
        drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
        break;
      case CONTEXTITEM_NUCLNAME:
        drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY_NUCLNAME]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
        break;
      case CONTEXTITEM_NUCLINFO:
        drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY_NUCLINFO]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
        break;
      case CONTEXTITEM_CHART_PROPERTY:
        switch(state->chartView){
          case CHARTVIEW_HALFLIFE:
            if(state->ds.useLifetimes){
              SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_LIFETIME]]);
              drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            }else{
              SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_HALFLIFE]]);
              drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            }
            break;
          case CHARTVIEW_DECAYMODE:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_DECAYMODE]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_2PLUS:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_R42:
            SDL_snprintf(tmpStr,32,"%s R₄₂",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_BETA2:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BETA2]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_SPIN:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_SPIN]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_PARITY:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_PARITY]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_BEA:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BEA]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_SN:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_SN]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_SP:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_SP]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_QALPHA:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_QALPHA]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_QBETAMINUS:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_QBETAMNUS]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_NUMLVLS:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_NUMLVLS]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          case CHARTVIEW_UNKNOWN_ENERGY:
            SDL_snprintf(tmpStr,32,"%s %s",dat->strings[dat->locStringIDs[LOCSTR_CONTEXT_COPY]],dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_UNKNOWN_ENERGY]]);
            drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (0.4f + (float)i)*CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

}

//draw some stats, ie. FPS overlay and further diagnostic info
void drawPerformanceStats(const ui_theme_rules *restrict uirules, const app_state *restrict state, const thread_manager_state *restrict tms, resource_data *restrict rdat, const float deltaTime){

  //draw background
  SDL_FRect perfOvRect;
  perfOvRect.w = (628.0f*rdat->uiScale);
  perfOvRect.h = (156.0f*rdat->uiScale);
  perfOvRect.x = (CHART_AXIS_DEPTH*rdat->uiScale);
  perfOvRect.y = 0.0f;
  
  SDL_SetRenderDrawColor(rdat->renderer,255,255,255,150);
  SDL_RenderFillRect(rdat->renderer,&perfOvRect);

  //draw text
  char txtStr[256];
  SDL_snprintf(txtStr,256,"Debug stats (press P to toggle this display)");
  drawText(rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR,blackCol8Bit,FONTSIZE_NORMAL_BOLD,txtStr);
  SDL_snprintf(txtStr,256,"UI scale: %0.2f, DPI scale: %0.2f, Resolution: %u x %u (logical), %u x %u (actual)",(double)rdat->uiScale,(double)rdat->uiDPIScale,state->ds.windowXRes,state->ds.windowYRes,state->ds.windowXRenderRes,state->ds.windowYRenderRes);
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"Zoom scale: %4.1f, UI status: %u",(double)state->ds.chartZoomScale,state->uiState);
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+2*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"Chart position: [%3.2f %3.2f], Selected nuclide: %4u",(double)state->ds.chartPosX,(double)state->ds.chartPosY,state->chartSelectedNucl);
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+3*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"Active threads: %2u, Selectable strings: %3u",tms->numThreads,state->tss.numSelStrs);
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+4*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"FPS: %4.1f",1.0/((double)deltaTime));
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+5*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"Frame time (ms): %4.3f",(double)(deltaTime*1000.0f));
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+6*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
}

//meta-function which draws any UI menus, if applicable
void drawUI(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){

  float yOffset = 0.0f;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_CHART_FADEIN)){
    yOffset = (-50.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHART_FADEIN]/(UI_ANIM_LENGTH)));
  }

  //draw background
  drawFlatBG(&state->ds,rdat,dat->rules.themeRules.bgCol);

  //draw chart of nuclides below everything else
  if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTOFNUCLIDES)){
    drawChartOfNuclides(dat,state,rdat);
    if(rdat->ssdat.takingScreenshot != 1){
      if(state->chartView == CHARTVIEW_HALFLIFE){
        if(state->ds.useLifetimes){
          drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_LIFETIME]]);
        }else{
          drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_HALFLIFE]]);
        }
      }else if(state->chartView == CHARTVIEW_DECAYMODE){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_DECAYMODE]]);
      }else if(state->chartView == CHARTVIEW_2PLUS){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]]);
      }else if(state->chartView == CHARTVIEW_R42){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_R42]]);
      }else if(state->chartView == CHARTVIEW_BETA2){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BETA2]]);
      }else if(state->chartView == CHARTVIEW_SPIN){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_SPIN]]);
      }else if(state->chartView == CHARTVIEW_PARITY){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_PARITY]]);
      }else if(state->chartView == CHARTVIEW_BEA){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BEA]]);
      }else if(state->chartView == CHARTVIEW_SN){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_SN]]);
      }else if(state->chartView == CHARTVIEW_SP){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_SP]]);
      }else if(state->chartView == CHARTVIEW_QALPHA){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_QALPHA]]);
      }else if(state->chartView == CHARTVIEW_QBETAMINUS){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_QBETAMNUS]]);
      }else if(state->chartView == CHARTVIEW_NUMLVLS){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_NUMLVLS]]);
      }else if(state->chartView == CHARTVIEW_UNKNOWN_ENERGY){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_UNKNOWN_ENERGY]]);
      }
      drawIconButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_SEARCH_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_SEARCH_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_SEARCH_BUTTON],getHighlightState(state,UIELEM_SEARCH_BUTTON),1.0f,UIICON_SEARCH);
      drawIcon(&dat->rules.themeRules,rdat,(int16_t)(state->ds.uiElemPosX[UIELEM_ZOOMIN_BUTTON] - yOffset),state->ds.uiElemPosY[UIELEM_ZOOMIN_BUTTON],state->ds.uiElemWidth[UIELEM_ZOOMIN_BUTTON],getHighlightState(state,UIELEM_ZOOMIN_BUTTON),1.0f,UIICON_ZOOMIN);
      drawIcon(&dat->rules.themeRules,rdat,(int16_t)(state->ds.uiElemPosX[UIELEM_ZOOMOUT_BUTTON] - yOffset),state->ds.uiElemPosY[UIELEM_ZOOMOUT_BUTTON],state->ds.uiElemWidth[UIELEM_ZOOMOUT_BUTTON],getHighlightState(state,UIELEM_ZOOMOUT_BUTTON),1.0f,UIICON_ZOOMOUT);
    }
  }
  
  //draw info boxes
  if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX)){
    drawNuclInfoBox(dat,state,rdat,state->chartSelectedNucl);
  }else if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX)){
    drawNuclFullInfoBox(dat,state,rdat,state->chartSelectedNucl);
    if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_RXN_MENU)){
      drawRxnMenu(dat,state,rdat);
    }
  }
  
  if(rdat->ssdat.takingScreenshot != 1){
    //draw persistent button(s)
    drawIconButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_MENU_BUTTON],(int16_t)(state->ds.uiElemPosY[UIELEM_MENU_BUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_MENU_BUTTON],getHighlightState(state,UIELEM_MENU_BUTTON),1.0f,UIICON_MENU);

    //draw menus/panels etc.
    if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU)){
      drawPrimaryMenu(dat,state,rdat);
    }
    if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU)){
      drawChartViewMenu(dat,state,rdat);
    }
    if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_SEARCH_MENU)){
      updateSearchUIState(dat,state,rdat);
      drawSearchMenu(dat,state,rdat);
    }

    //draw modal dialogs
    if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_ABOUT_BOX)){
      drawAboutBox(dat,state,rdat);
    }else if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_PREFS_DIALOG)){
      drawPrefsDialog(dat,state,rdat);
      if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU)){
        drawUIScaleMenu(dat,state,rdat);
      }
    }

    //draw highlight over selected text
    drawTextSelHighlight(state,rdat);

    if(state->cms.numContextMenuItems > 0){
      drawContextMenu(dat,state,rdat);
    }

    if(state->ds.uiAnimPlaying & (1U << UIANIM_CHART_FADEIN)){
      
      SDL_FColor white = {1.0f,1.0f,1.0f,1.0f};
      white.a = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHART_FADEIN]/UI_ANIM_LENGTH));
      drawFlatBG(&state->ds,rdat,white);
    }
  }

  //all text is now drawn (and any applicable selection strings defined),
  //disable selection string updates (until re-enabled by some later event)
  state->tss.selStrsModifiable = 0;
  
}
