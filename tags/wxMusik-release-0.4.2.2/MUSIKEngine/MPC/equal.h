#ifndef XMMS_EQUAL_H
#define XMMS_EQUAL_H

#include <stdlib.h>

typedef struct {
    float  freq;                // Hz
    float  level;               // dB
} corr_elem_t;

typedef struct {
    const corr_elem_t*  tab;
    size_t              len;
} corr_t;

// loudspeakers
extern const corr_t  corr_sonusfaber_amati;

// headphones
extern const corr_t  corr_sennheiser_eh2200;
extern const corr_t  corr_sennheiser_hd25_sp;
extern const corr_t  corr_sennheiser_hd250_ii;
extern const corr_t  corr_sennheiser_hd265;
extern const corr_t  corr_sennheiser_hd270;
extern const corr_t  corr_sennheiser_hd433;
extern const corr_t  corr_sennheiser_hd451;
extern const corr_t  corr_sennheiser_hd490;
extern const corr_t  corr_sennheiser_hd495;
extern const corr_t  corr_sennheiser_hd500;
extern const corr_t  corr_sennheiser_hd570;
extern const corr_t  corr_sennheiser_hd575;
extern const corr_t  corr_sennheiser_hd580;
extern const corr_t  corr_sennheiser_hd590;
extern const corr_t  corr_sennheiser_hd600;

extern const corr_t  corr_grado_sr40;
extern const corr_t  corr_grado_sr60;
extern const corr_t  corr_grado_sr80;
extern const corr_t  corr_grado_sr125;
extern const corr_t  corr_grado_sr225;
extern const corr_t  corr_grado_sr325;
extern const corr_t  corr_grado_rs2;
extern const corr_t  corr_grado_rs1;

extern const corr_t  corr_beyer_dt131;
extern const corr_t  corr_beyer_dt231;
extern const corr_t  corr_beyer_dt331;
extern const corr_t  corr_beyer_dt770;
extern const corr_t  corr_beyer_dt831;
extern const corr_t  corr_beyer_dt931;
extern const corr_t  corr_beyer_dt990;
extern const corr_t  corr_beyer_dt250_250;

extern const corr_t  corr_akg_k55;
extern const corr_t  corr_akg_k66;
extern const corr_t  corr_akg_k70;
extern const corr_t  corr_akg_k100;
extern const corr_t  corr_akg_k141;
extern const corr_t  corr_akg_k240m;
extern const corr_t  corr_akg_k240_df;
extern const corr_t  corr_akg_k270s;
extern const corr_t  corr_akg_k301;
extern const corr_t  corr_akg_k401;
extern const corr_t  corr_akg_k501;

extern const corr_t  corr_koss_ur20;
extern const corr_t  corr_koss_ur30;
extern const corr_t  corr_koss_ksc35;
extern const corr_t  corr_koss_sporta_pro;
extern const corr_t  corr_koss_porta_pro;
extern const corr_t  corr_koss_qz_99;
extern const corr_t  corr_koss_r100;

#endif /* XMMS_EQUAL_H */

/* end of equal.h */
