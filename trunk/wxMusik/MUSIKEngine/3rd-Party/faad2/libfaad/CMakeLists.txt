INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(myfaad2 bits.c     drc.c       hcr.c         lt_predict.c
		    output.c     pulse.c    sbr_e_nf.c   sbr_huff.c     specrec.c   syntax.c
		    cfft.c     drm_dec.c   huffman.c     mdct.c        pns.c        rvlc.c
		    sbr_fbt.c    sbr_qmf.c      ssr.c       tns.c
		    common.c   error.c     ic_predict.c  mp4.c         ps_dec.c     sbr_dct.c
		    sbr_hfadj.c  sbr_syntax.c   ssr_fb.c
		    decoder.c  filtbank.c  is.c          ms.c          ps_syntax.c  sbr_dec.c
		    sbr_hfgen.c  sbr_tf_grid.c  ssr_ipqf.c)