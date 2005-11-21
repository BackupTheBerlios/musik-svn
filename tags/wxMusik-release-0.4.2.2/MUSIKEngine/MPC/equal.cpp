
#include "equal.h"

#ifdef _WIN32
# pragma warning ( disable : 4305 )
#endif

//---------------------------------------------------------------------------------------------------------------------------------
static const corr_elem_t  _corrtab_akg_k141          [] = {
#include "akg_k141.h"
};
const corr_t              corr_akg_k141                 = { _corrtab_akg_k141        , sizeof(_corrtab_akg_k141        )/sizeof(*_corrtab_akg_k141        ) };
//---------------------------------------------------------------------------------------------------------------------------------
static const corr_elem_t  _corrtab_akg_k401          [] = {
#include "akg_k401.h"
};
const corr_t              corr_akg_k401                 = { _corrtab_akg_k401        , sizeof(_corrtab_akg_k401        )/sizeof(*_corrtab_akg_k401        ) };
//---------------------------------------------------------------------------------------------------------------------------------
static const corr_elem_t  _corrtab_akg_k501          [] = {
#include "akg_k501.h"
};
const corr_t              corr_akg_k501                 = { _corrtab_akg_k501        , sizeof(_corrtab_akg_k501        )/sizeof(*_corrtab_akg_k501        ) };
//---------------------------------------------------------------------------------------------------------------------------------
static const corr_elem_t   _corrtab_sennheiser_hd580 [] = {
#include "sh_hd580.h"
};
const corr_t               corr_sennheiser_hd580        = { _corrtab_sennheiser_hd580, sizeof(_corrtab_sennheiser_hd580)/sizeof(*_corrtab_sennheiser_hd580) };
//---------------------------------------------------------------------------------------------------------------------------------
static const corr_elem_t   _corrtab_sennheiser_hd600 [] = {
#include "sh_hd600.h"
};
const corr_t               corr_sennheiser_hd600        = { _corrtab_sennheiser_hd600, sizeof(_corrtab_sennheiser_hd600)/sizeof(*_corrtab_sennheiser_hd600) };
//---------------------------------------------------------------------------------------------------------------------------------
static const corr_elem_t  _corrtab_sonusfaber_amati  [] = {
#include "sf_amati.h"
};
const corr_t               corr_sonusfaber_amati        = { _corrtab_sonusfaber_amati, sizeof(_corrtab_sonusfaber_amati)/sizeof(*_corrtab_sonusfaber_amati) };
//---------------------------------------------------------------------------------------------------------------------------------

/* end of equal.c */
