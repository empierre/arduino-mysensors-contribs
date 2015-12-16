/*
  Air Quality Module DS-HCHO
  
  author: terryoy
  email: terry.ouyang@gmail.com
*/
#include <math.h>

/* Request data definition */
enum Command {
  CMD_QUERY = 0x01,
  CMD_CLEAR = 0x02,
  CMD_SPAN = 0x03,
  CMD_FACTORY = 0x04,
};

typedef struct {
  char begin_1 = 0x42;
  char begin_2 = 0x4d;
  char cmd = CMD_QUERY;
  char dhh = 0;
  char dll = 0;
  char lrch = 0;
  char lrcl = 0;
  
} AirCommand;

/* Response data definition */

enum AirType {
  AIRTYPE_NO_SENSOR = 0x00,
  AIRTYPE_CO = 0x01,
  AIRTYPE_H2S = 0x02, 
  AIRTYPE_CH4 = 0x03,
  AIRTYPE_CL2 = 0x04,
  AIRTYPE_HCL = 0x05,
  AIRTYPE_F2 = 0x06,
  AIRTYPE_HF = 0x07,
  AIRTYPE_NH3 = 0x08,
  AIRTYPE_HCN = 0x09,
  AIRTYPE_PH3 = 0x0a,
  AIRTYPE_NO = 0x0b,
  AIRTYPE_NO2 = 0x0c, 
  AIRTYPE_O3 = 0x0d, 
  AIRTYPE_O2 = 0x0e,
  AIRTYPE_SO2 = 0x0f,
  AIRTYPE_CLO2 = 0x10,
  AIRTYPE_COCL2 = 0x11,
  AIRTYPE_PH3_ = 0x12, // duplicated with 0x0a
  AIRTYPE_SIH4 = 0x13,
  AIRTYPE_HCHO = 0x14,
  AIRTYPE_CO2 = 0x15,
  AIRTYPE_VOC = 0x16,
  AIRTYPE_ETO = 0x17,
  AIRTYPE_C2H4 = 0x18,
  AIRTYPE_C2H2 = 0x19,
  AIRTYPE_SF6 = 0x1a,
  AIRTYPE_ASH3 = 0x1b,
  AIRTYPE_H2 = 0x1c,
  AIRTYPE_TOX1 = 0x1d,
  AIRTYPE_TOX2 = 0x1e,
  AIRTYPE_GAS_FLOW = 0x1f, // L/M
  AIRTYPE_BATTERY = 0x20,
};

enum MeasureUnit {
  MU_PPM = 0x01,
  MU_VOL = 0x02,
  MU_LEL = 0x03,
  MU_PPB = 0x04,
  MU_MG_M3 = 0x05,
};

enum Equivalent {
  EQV_ONE = 0x01,
  EQV_TEN = 0x02,
  EQV_HUNDRED = 0x03,
  EQV_THOUSAND = 0x04,
};

typedef struct {
  char begin_1 = 0x42;
  char begin_2 = 0x4d;
  char length = 0x08;
  char air_type = 0x00;
  char unit = 0x00;
  char vh = 0x00; //
  char dhh = 0x00;
  char dll = 0x00;
  char lrch = 0x00;
  char lrcl = 0x00;
} AirResponse; // length 10

void parse_air_response(AirResponse *response, char *raw) {
  response->begin_1 = raw[0];
  response->begin_2 = raw[1];
  response->length = raw[2];
  response->air_type = raw[3];
  response->unit = raw[4];
  response->vh = raw[5];
  response->dhh = raw[6];
  response->dll = raw[7];
  response->lrch = raw[8];
  response->lrcl = raw[9];
}

char* get_unit_display(AirResponse response) {
  switch(response.unit) {
    case MU_PPM:
      return "ppm";
    case MU_VOL:
      return "VOL";
    case MU_LEL:
      return "LEL";
    case MU_PPB:
      return "Ppb";
    case MU_MG_M3:
      return "Mg/m3";
    default:
      return "";
  }
}

float get_read_value(AirResponse response) {
  float equivalent = pow(10, response.vh) / 10;
  int read_value = (int)response.dhh << 8 | (int)response.dll;
  return read_value / equivalent;
}

