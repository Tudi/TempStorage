#define _GNU_SOURCE

#include "ndpi_config.h"

#ifdef __linux__
#include <sched.h>
#endif

#include "ndpi_api.h"
#include "third_party/include/uthash.h"
#include "third_party/include/ahocorasick.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <float.h> /* FLT_EPSILON */
#ifdef WIN32
#include <winsock2.h> /* winsock.h is included automatically */
#include <windows.h>
#include <ws2tcpip.h>
#include <process.h>
#include <io.h>
#else
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/mman.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <search.h>
#include <pcap.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>  
#include <assert.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _MSC_BUILD
#include <libgen.h>
#endif
#include <errno.h>

#include "reader_util.h"
#include "ndpiReader.h"

void* processing_thread(void* _thread_id);
void sigproc(int sig);
void configurePcapHandle(pcap_t* pcap_handle);
int getNextPcapFileFromPlaylist(u_int16_t thread_id, char filename[], u_int32_t filename_len);
void printResults(u_int64_t processing_time_usec, u_int64_t setup_time_usec);


/* ***************************************************** */
/* *********************************************** */

#if 0
static void binUnitTest() {
    struct ndpi_bin* bins, b0, b1;
    u_int8_t num_bins = 32;
    u_int8_t num_points = 24;
    u_int32_t i, j;
    u_int8_t num_clusters = 3;
    u_int16_t cluster_ids[256];
    char out_buf[128];

    srand(time(NULL));

    assert((bins = (struct ndpi_bin*)ndpi_malloc(sizeof(struct ndpi_bin) * num_bins)) != NULL);

    for (i = 0; i < num_bins; i++) {
        ndpi_init_bin(&bins[i], ndpi_bin_family8, num_points);

        for (j = 0; j < num_points; j++)
            ndpi_set_bin(&bins[i], j, rand() % 0xFF);

        ndpi_normalize_bin(&bins[i]);
    }

    ndpi_cluster_bins(bins, num_bins, num_clusters, cluster_ids, NULL);

    for (j = 0; j < num_clusters; j++) {
        if (verbose) printf("\n");

        for (i = 0; i < num_bins; i++) {
            if (cluster_ids[i] == j) {
                if (verbose)
                    printf("[%u] %s\n", cluster_ids[i],
                        ndpi_print_bin(&bins[i], 0, out_buf, sizeof(out_buf)));
            }
        }
    }
    // printf("Similarity: %f\n\n", ndpi_bin_similarity(&b1, &b2, 1));

    for (i = 0; i < num_bins; i++)
        ndpi_free_bin(&bins[i]);

    ndpi_free(bins);

    /* ************************ */

    ndpi_init_bin(&b0, ndpi_bin_family8, 16);
    ndpi_init_bin(&b1, ndpi_bin_family8, 16);

    ndpi_set_bin(&b0, 1, 100);
    ndpi_set_bin(&b1, 1, 100);

    printf("Similarity: %f\n\n", ndpi_bin_similarity(&b0, &b1, 1));

    ndpi_free_bin(&b0), ndpi_free_bin(&b1);

    // exit(0);
}
#endif

/* *********************************************** */
#ifndef DEBUG_TRACE

static void dgaUnitTest() {
    const char* dga[] = {
        //"www.lbjamwptxz.com",
        "www.l54c2e21e80ba5471be7a8402cffb98768.so",
        "www.wdd7ee574106a84807a601beb62dd851f0.hk",
        "www.jaa12148a5831a5af92aa1d8fe6059e276.ws",
        "www.e6r5p57kbafwrxj3plz.com",
        // "grdawgrcwegpjaoo.eu",
        "www.mcfpeqbotiwxfxqu.eu",
        "www.adgxwxhqsegnrsih.eu",
        NULL
    };

    const char* non_dga[] = {
      "mail.100x100design.com",
      "cdcvps.cloudapps.cisco.com",
      "vcsa.vmware.com",
      "mz.gov.pl",
      "zoomam104zc.zoom.us",
      "5CI_DOMBIN",
      "ALICEGATE",
      "BOWIE",
      "D002465",
      "DESKTOP-RB5T12G",
      "ECI_DOM",
      "ECI_DOMA",
      "ECI_DOMAIN",
      "ENDIAN-PC",
      "GFILE",
      "GIOVANNI-PC",
      "GUNNAR",
      "ISATAP",
      "LAB111",
      "LP-RKERUR-OSX",
      "LUCAS-IMAC",
      "LUCASMACBOOKPRO",
      "MACBOOKAIR-E1D0",
      //"MDJR98",
      "NASFILE",
      "SANJI-LIFEBOOK-",
      "SC.ARRANCAR.ORG",
      "WORKG",
      "WORKGROUP",
      "XSTREAM_HY",
      "__MSBROWSE__",
      "mqtt.facebook.com",
      NULL
    };
    int debug = 0, i;
    NDPI_PROTOCOL_BITMASK all;
    struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);

    assert(ndpi_str != NULL);

    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);

    ndpi_finalize_initialization(ndpi_str);

    assert(ndpi_str != NULL);

    for (i = 0; non_dga[i] != NULL; i++) {
        if (debug) printf("Checking non DGA %s\n", non_dga[i]);
        assert(ndpi_check_dga_name(ndpi_str, NULL, (char*)non_dga[i], 1, 1, 0) == 0);
    }

    for (i = 0; dga[i] != NULL; i++) {
        if (debug) printf("Checking DGA %s\n", non_dga[i]);
        assert(ndpi_check_dga_name(ndpi_str, NULL, (char*)dga[i], 1, 1, 0) == 1);
    }

    ndpi_exit_detection_module(ndpi_str);
}

/* *********************************************** */

static void hllUnitTest() {
    struct ndpi_hll h;
    u_int8_t bits = 8; /* >= 4, <= 16 */
    u_int32_t i;

    assert(ndpi_hll_init(&h, bits) == 0);

    for (i = 0; i < 21320; i++)
        ndpi_hll_add_number(&h, i);

    /* printf("Count estimate: %f\n", ndpi_hll_count(&h)); */

    ndpi_hll_destroy(&h);
}

/* *********************************************** */

static void bitmapUnitTest() {
    u_int32_t val, i, j;
    u_int64_t val64;

    /* With a 32 bit integer */
    for (i = 0; i < 32; i++) {
        NDPI_ZERO_BIT(val);
        NDPI_SET_BIT(val, i);

        assert(NDPI_ISSET_BIT(val, i));

        for (j = 0; j < 32; j++) {
            if (j != i) {
                assert(!NDPI_ISSET_BIT(val, j));
            }
        }
    }

    /* With a 64 bit integer */
    for (i = 0; i < 64; i++) {
        NDPI_ZERO_BIT(val64);
        NDPI_SET_BIT(val64, i);

        assert(NDPI_ISSET_BIT(val64, i));

        for (j = 0; j < 64; j++) {
            if (j != i) {
                assert(!NDPI_ISSET_BIT(val64, j));
            }
        }
    }
}

/* *********************************************** */

void automataUnitTest() {
    void* automa = ndpi_init_automa();

    assert(automa);
    assert(ndpi_add_string_to_automa(automa, ndpi_strdup("hello")) == 0);
    assert(ndpi_add_string_to_automa(automa, ndpi_strdup("world")) == 0);
    ndpi_finalize_automa(automa);
    assert(ndpi_match_string(automa, "This is the wonderful world of nDPI") == 1);
    ndpi_free_automa(automa);
}

/* *********************************************** */

void automataDomainsUnitTest() {
    void* automa = ndpi_init_automa_domain();

    assert(automa);
    assert(ndpi_add_string_to_automa(automa, ndpi_strdup("wikipedia.it")) == 0);
    ndpi_finalize_automa(automa);
    assert(ndpi_match_string(automa, "wikipedia.it") == 1);
    assert(ndpi_match_string(automa, "foo.wikipedia.it") == 1);
    assert(ndpi_match_string(automa, "foowikipedia.it") == 0);
    assert(ndpi_match_string(automa, "foowikipedia") == 0);
    assert(ndpi_match_string(automa, "-wikipedia.it") == 0);
    assert(ndpi_match_string(automa, "foo-wikipedia.it") == 0);
    assert(ndpi_match_string(automa, "wikipedia.it.com") == 0);
    ndpi_free_automa(automa);

    automa = ndpi_init_automa_domain();
    assert(automa);
    assert(ndpi_add_string_to_automa(automa, ndpi_strdup("wikipedia.")) == 0);
    ndpi_finalize_automa(automa);
    assert(ndpi_match_string(automa, "wikipedia.it") == 1);
    assert(ndpi_match_string(automa, "foo.wikipedia.it") == 1);
    assert(ndpi_match_string(automa, "foowikipedia.it") == 0);
    assert(ndpi_match_string(automa, "foowikipedia") == 0);
    assert(ndpi_match_string(automa, "-wikipedia.it") == 0);
    assert(ndpi_match_string(automa, "foo-wikipedia.it") == 0);
    assert(ndpi_match_string(automa, "wikipediafoo") == 0);
    assert(ndpi_match_string(automa, "wikipedia.it.com") == 1);
    ndpi_free_automa(automa);

    automa = ndpi_init_automa_domain();
    assert(automa);
    assert(ndpi_add_string_to_automa(automa, ndpi_strdup("-buy.itunes.apple.com")) == 0);
    ndpi_finalize_automa(automa);
    assert(ndpi_match_string(automa, "buy.itunes.apple.com") == 0);
    assert(ndpi_match_string(automa, "p53-buy.itunes.apple.com") == 1);
    assert(ndpi_match_string(automa, "p53buy.itunes.apple.com") == 0);
    assert(ndpi_match_string(automa, "foo.p53-buy.itunes.apple.com") == 1);
    ndpi_free_automa(automa);
}

#endif

/* *********************************************** */

// #define RUN_DATA_ANALYSIS_THEN_QUIT 1

void analyzeUnitTest() {
    struct ndpi_analyze_struct* s = ndpi_alloc_data_analysis(32);
    u_int32_t i;

    for (i = 0; i < 256; i++) {
        ndpi_data_add_value(s, rand() * (u_int64_t)i);
        // ndpi_data_add_value(s, i+1);
    }

    // ndpi_data_print_window_values(s);

#ifdef RUN_DATA_ANALYSIS_THEN_QUIT
    printf("Average: [all: %f][window: %f]\n",
        ndpi_data_average(s), ndpi_data_window_average(s));
    printf("Entropy: %f\n", ndpi_data_entropy(s));

    printf("Min/Max: %u/%u\n",
        ndpi_data_min(s), ndpi_data_max(s));
#endif

    ndpi_free_data_analysis(s, 1);

#ifdef RUN_DATA_ANALYSIS_THEN_QUIT
    exit(0);
#endif
}


/* *********************************************** */

void analysisUnitTest() {
    struct ndpi_analyze_struct* s = ndpi_alloc_data_analysis(32);
    u_int32_t i;

    for (i = 0; i < 256; i++)
        ndpi_data_add_value(s, i);

    if (0) {
        ndpi_data_print_window_values(s);
        printf("Average: [all: %f][window: %f]\n", ndpi_data_average(s), ndpi_data_window_average(s));
        printf("Entropy: %f\n", ndpi_data_entropy(s));
        printf("StdDev:  %f\n", ndpi_data_stddev(s));
        printf("Min/Max: %llu/%llu\n",
            (unsigned long long int)ndpi_data_min(s),
            (unsigned long long int)ndpi_data_max(s));
    }

    ndpi_free_data_analysis(s, 1);
}

/* *********************************************** */

void rsiUnitTest() {
    struct ndpi_rsi_struct s;
    unsigned int v[] = {
      31,
      87,
      173,
      213,
      223,
      230,
      238,
      245,
      251,
      151,
      259,
      261,
      264,
      264,
      270,
      273,
      288,
      288,
      304,
      304,
      350,
      384,
      423,
      439,
      445,
      445,
      445,
      445
    };

    u_int i, n = sizeof(v) / sizeof(unsigned int);
    u_int debug = 0;

    assert(ndpi_alloc_rsi(&s, 8) == 0);

    for (i = 0; i < n; i++) {
        float rsi = ndpi_rsi_add_value(&s, v[i]);


        if (debug)
            printf("%2d) RSI = %f\n", i, rsi);
    }

    ndpi_free_rsi(&s);
}

/* *********************************************** */

void hashUnitTest() {
    ndpi_str_hash* h;
    char* const dict[] = { "hello", "world", NULL };
    u_int16_t i;

    assert(ndpi_hash_init(&h) == 0);
    assert(h == NULL);

    for (i = 0; dict[i] != NULL; i++) {
        u_int8_t l = strlen(dict[i]);
        u_int16_t v;

        assert(ndpi_hash_add_entry(&h, dict[i], l, i) == 0);
        assert(ndpi_hash_find_entry(h, dict[i], l, &v) == 0);
        assert(v == i);
    }

    ndpi_hash_free(&h);
    assert(h == NULL);
}

/* *********************************************** */

void hwUnitTest() {
    struct ndpi_hw_struct hw;
    double v[] = { 10, 14, 8, 25, 16, 22, 14, 35, 15, 27, 218, 40, 28, 40, 25, 65 };
    u_int i, j, num = sizeof(v) / sizeof(double);
    u_int num_learning_points = 2;
    u_int8_t trace = 0;

    for (j = 0; j < 2; j++) {
        assert(ndpi_hw_init(&hw, num_learning_points, j /* 0=multiplicative, 1=additive */, 0.9, 0.9, 0.1, 0.05) == 0);

        if (trace)
            printf("\nHolt-Winters %s method\n", (j == 0) ? "multiplicative" : "additive");

        for (i = 0; i < num; i++) {
            double prediction, confidence_band;
            double lower, upper;
            int rc = ndpi_hw_add_value(&hw, v[i], &prediction, &confidence_band);

            lower = prediction - confidence_band, upper = prediction + confidence_band;

            if (trace)
                printf("%2u)\t%.3f\t%.3f\t%.3f\t%.3f\t %s [%.3f]\n", i, v[i], prediction, lower, upper,
                    ((rc == 0) || ((v[i] >= lower) && (v[i] <= upper))) ? "OK" : "ANOMALY",
                    confidence_band);
        }

        ndpi_hw_free(&hw);
    }
}

/* *********************************************** */

void hwUnitTest2() {
    struct ndpi_hw_struct hw;
    u_int8_t trace = 1;
    double v[] = {
      31.908466339111,
      87.339714050293,
      173.47660827637,
      213.92568969727,
      223.32124328613,
      230.60134887695,
      238.09457397461,
      245.8137512207,
      251.09228515625,
      251.09228515625,
      259.21997070312,
      261.98754882812,
      264.78540039062,
      264.78540039062,
      270.47451782227,
      173.3671875,
      288.34222412109,
      288.34222412109,
      304.24795532227,
      304.24795532227,
      350.92227172852,
      384.54431152344,
      423.25942993164,
      439.43322753906,
      445.05981445312,
      445.05981445312,
      445.05981445312,
      445.05981445312
    };
    u_int num_learning_points = 1;
    u_int i, num = sizeof(v) / sizeof(double);
    float alpha = 0.9, beta = 0.5, gamma = 1;
    FILE* fd = fopen("/tmp/result.csv", "w");

    assert(ndpi_hw_init(&hw, num_learning_points, 0 /* 0=multiplicative, 1=additive */,
        alpha, beta, gamma, 0.05) == 0);

    if (trace) {
        printf("\nHolt-Winters [alpha: %.1f][beta: %.1f][gamma: %.1f]\n", alpha, beta, gamma);

        if (fd)
            fprintf(fd, "index;value;prediction;lower;upper;anomaly\n");
    }

    for (i = 0; i < num; i++) {
        double prediction, confidence_band;
        double lower, upper;
        int rc = ndpi_hw_add_value(&hw, v[i], &prediction, &confidence_band);

        lower = prediction - confidence_band, upper = prediction + confidence_band;

        if (trace) {
            printf("%2u)\t%12.3f\t%.3f\t%12.3f\t%12.3f\t %s [%.3f]\n", i, v[i], prediction, lower, upper,
                ((rc == 0) || ((v[i] >= lower) && (v[i] <= upper))) ? "OK" : "ANOMALY",
                confidence_band);

            if (fd)
                fprintf(fd, "%u;%.0f;%.0f;%.0f;%.0f;%s\n",
                    i, v[i], prediction, lower, upper,
                    ((rc == 0) || ((v[i] >= lower) && (v[i] <= upper))) ? "OK" : "ANOMALY");
        }
    }

    if (fd) fclose(fd);

    ndpi_hw_free(&hw);

    //exit(0);
}

/* *********************************************** */

void sesUnitTest() {
    struct ndpi_ses_struct ses;
    u_int8_t trace = 0;
    double v[] = {
      31.908466339111,
      87.339714050293,
      173.47660827637,
      213.92568969727,
      223.32124328613,
      230.60134887695,
      238.09457397461,
      245.8137512207,
      251.09228515625,
      251.09228515625,
      259.21997070312,
      261.98754882812,
      264.78540039062,
      264.78540039062,
      270.47451782227,
      173.3671875,
      288.34222412109,
      288.34222412109,
      304.24795532227,
      304.24795532227,
      350.92227172852,
      384.54431152344,
      423.25942993164,
      439.43322753906,
      445.05981445312,
      445.05981445312,
      445.05981445312,
      445.05981445312
    };
    u_int i, num = sizeof(v) / sizeof(double);
    float alpha = 0.9;
    FILE* fd = fopen("/tmp/ses_result.csv", "w");

    assert(ndpi_ses_init(&ses, alpha, 0.05) == 0);
    ndpi_ses_reset(&ses);

    if (trace) {
        printf("\nSingle Exponential Smoothing [alpha: %.1f]\n", alpha);

        if (fd)
            fprintf(fd, "index;value;prediction;lower;upper;anomaly\n");
    }

    for (i = 0; i < num; i++) {
        double prediction, confidence_band;
        double lower, upper;
        int rc = ndpi_ses_add_value(&ses, v[i], &prediction, &confidence_band);

        lower = prediction - confidence_band, upper = prediction + confidence_band;

        if (trace) {
            printf("%2u)\t%12.3f\t%.3f\t%12.3f\t%12.3f\t %s [%.3f]\n", i, v[i], prediction, lower, upper,
                ((rc == 0) || ((v[i] >= lower) && (v[i] <= upper))) ? "OK" : "ANOMALY",
                confidence_band);

            if (fd)
                fprintf(fd, "%u;%.0f;%.0f;%.0f;%.0f;%s\n",
                    i, v[i], prediction, lower, upper,
                    ((rc == 0) || ((v[i] >= lower) && (v[i] <= upper))) ? "OK" : "ANOMALY");
        }
    }

    if (fd) fclose(fd);

    ndpi_ses_fitting(v, num, &alpha); /* Compute the best alpha */
}

/* *********************************************** */

void desUnitTest() {
    struct ndpi_des_struct des;
    u_int8_t trace = 0;
    double v[] = {
      31.908466339111,
      87.339714050293,
      173.47660827637,
      213.92568969727,
      223.32124328613,
      230.60134887695,
      238.09457397461,
      245.8137512207,
      251.09228515625,
      251.09228515625,
      259.21997070312,
      261.98754882812,
      264.78540039062,
      264.78540039062,
      270.47451782227,
      173.3671875,
      288.34222412109,
      288.34222412109,
      304.24795532227,
      304.24795532227,
      350.92227172852,
      384.54431152344,
      423.25942993164,
      439.43322753906,
      445.05981445312,
      445.05981445312,
      445.05981445312,
      445.05981445312
    };
    u_int i, num = sizeof(v) / sizeof(double);
    float alpha = 0.9, beta = 0.5;
    FILE* fd = fopen("/tmp/des_result.csv", "w");

    assert(ndpi_des_init(&des, alpha, beta, 0.05) == 0);
    ndpi_des_reset(&des);

    if (trace) {
        printf("\nDouble Exponential Smoothing [alpha: %.1f][beta: %.1f]\n", alpha, beta);

        if (fd)
            fprintf(fd, "index;value;prediction;lower;upper;anomaly\n");
    }

    for (i = 0; i < num; i++) {
        double prediction, confidence_band;
        double lower, upper;
        int rc = ndpi_des_add_value(&des, v[i], &prediction, &confidence_band);

        lower = prediction - confidence_band, upper = prediction + confidence_band;

        if (trace) {
            printf("%2u)\t%12.3f\t%.3f\t%12.3f\t%12.3f\t %s [%.3f]\n", i, v[i], prediction, lower, upper,
                (rc == 0) ? "LEARNING" : (((v[i] >= lower) && (v[i] <= upper)) ? "OK" : "ANOMALY"),
                confidence_band);

            if (fd)
                fprintf(fd, "%u;%.0f;%.0f;%.0f;%.0f;%s\n",
                    i, v[i], prediction, lower, upper,
                    ((rc == 0) || ((v[i] >= lower) && (v[i] <= upper))) ? "OK" : "ANOMALY");
        }
    }

    if (fd) fclose(fd);

    ndpi_des_fitting(v, num, &alpha, &beta); /* Compute the best alpha/beta */
}

/* *********************************************** */

void desUnitStressTest() {
    struct ndpi_des_struct des;
    u_int8_t trace = 1;
    u_int i;
    float alpha = 0.9, beta = 0.5;
    double init_value = time(NULL) % 1000;

    assert(ndpi_des_init(&des, alpha, beta, 0.05) == 0);
    ndpi_des_reset(&des);

    if (trace) {
        printf("\nDouble Exponential Smoothing [alpha: %.1f][beta: %.1f]\n", alpha, beta);
    }

    for (i = 0; i < 512; i++) {
        double prediction, confidence_band;
        double lower, upper;
        double value = init_value + rand() % 25;
        int rc = ndpi_des_add_value(&des, value, &prediction, &confidence_band);

        lower = prediction - confidence_band, upper = prediction + confidence_band;

        if (trace) {
            printf("%2u)\t%12.3f\t%.3f\t%12.3f\t%12.3f\t %s [%.3f]\n", i, value, prediction, lower, upper,
                ((rc == 0) || ((value >= lower) && (value <= upper))) ? "OK" : "ANOMALY",
                confidence_band);
        }
    }
}

/* *********************************************** */

void hwUnitTest3() {
    struct ndpi_hw_struct hw;
    u_int num_learning_points = 3;
    u_int8_t trace = 1;
    double v[] = {
      10,
      14,
      8,
      25,
      16,
      22,
      14,
      35,
      15,
      27,
      18,
      40,
      28,
      40,
      25,
      65,
    };
    u_int i, num = sizeof(v) / sizeof(double);
    float alpha = 0.5, beta = 0.5, gamma = 0.1;
    assert(ndpi_hw_init(&hw, num_learning_points, 0 /* 0=multiplicative, 1=additive */, alpha, beta, gamma, 0.05) == 0);
    ndpi_hw_reset(&hw);

    if (trace)
        printf("\nHolt-Winters [alpha: %.1f][beta: %.1f][gamma: %.1f]\n", alpha, beta, gamma);

    for (i = 0; i < num; i++) {
        double prediction, confidence_band;
        double lower, upper;
        int rc = ndpi_hw_add_value(&hw, v[i], &prediction, &confidence_band);

        lower = prediction - confidence_band, upper = prediction + confidence_band;

        if (trace)
            printf("%2u)\t%12.3f\t%.3f\t%12.3f\t%12.3f\t %s [%.3f]\n",
                i, v[i], prediction, lower, upper,
                ((rc == 0) || ((v[i] >= lower) && (v[i] <= upper))) ? "OK" : "ANOMALY",
                confidence_band);
    }

    ndpi_hw_free(&hw);
}

/* *********************************************** */

void jitterUnitTest() {
    struct ndpi_jitter_struct jitter;
    float v[] = { 10, 14, 8, 25, 16, 22, 14, 35, 15, 27, 218, 40, 28, 40, 25, 65 };
    u_int i, num = sizeof(v) / sizeof(float);
    u_int num_learning_points = 4;
    u_int8_t trace = 0;

    assert(ndpi_jitter_init(&jitter, num_learning_points) == 0);

    for (i = 0; i < num; i++) {
        float rc = ndpi_jitter_add_value(&jitter, v[i]);

        if (trace)
            printf("%2u)\t%.3f\t%.3f\n", i, v[i], rc);
    }

    ndpi_jitter_free(&jitter);
}

/* *********************************************** */

void compressedBitmapUnitTest() {
    ndpi_bitmap* b = ndpi_bitmap_alloc(), * b1;
    u_int i, trace = 0;
    size_t ser;
    char* buf;
    ndpi_bitmap_iterator* it;
    u_int64_t value;

    for (i = 0; i < 1000; i++) {
        u_int32_t v = rand();

        if (trace) printf("%u ", v);
        ndpi_bitmap_set(b, v);
        assert(ndpi_bitmap_isset(b, v));
    }

    if (trace) printf("\n");

    ser = ndpi_bitmap_serialize(b, &buf);
    assert(ser > 0);

    if (trace) printf("len: %u\n", (unsigned int)ser);
    b1 = ndpi_bitmap_deserialize(buf, ser);
    assert(b1);

    assert((it = ndpi_bitmap_iterator_alloc(b)));
    while (ndpi_bitmap_iterator_next(it, &value)) {
        if (trace) printf("%lu ", (unsigned long)value);
    }

    if (trace) printf("\n");
    ndpi_bitmap_iterator_free(it);

    ndpi_free(buf);
    ndpi_bitmap_free(b);
    ndpi_bitmap_free(b1);
}

/* *********************************************** */

void strtonumUnitTest() {
    const char* errstrp;

    assert(ndpi_strtonum("0", -10, +10, &errstrp, 10) == 0);
    assert(errstrp == NULL);
    assert(ndpi_strtonum("0", +10, -10, &errstrp, 10) == 0);
    assert(errstrp != NULL);
    assert(ndpi_strtonum("  -11  ", -10, +10, &errstrp, 10) == 0);
    assert(errstrp != NULL);
    assert(ndpi_strtonum("  -11  ", -100, +100, &errstrp, 10) == -11);
    assert(errstrp == NULL);
    assert(ndpi_strtonum("123abc", LLONG_MIN, LLONG_MAX, &errstrp, 10) == 123);
    assert(errstrp == NULL);
    assert(ndpi_strtonum("123abc", LLONG_MIN, LLONG_MAX, &errstrp, 16) == 0x123abc);
    assert(errstrp == NULL);
    assert(ndpi_strtonum("  0x123abc", LLONG_MIN, LLONG_MAX, &errstrp, 16) == 0x123abc);
    assert(errstrp == NULL);
    assert(ndpi_strtonum("ghi", -10, +10, &errstrp, 10) == 0);
    assert(errstrp != NULL);
}

/* *********************************************** */

void strlcpyUnitTest() {
    // Test empty string
    char dst_empty[10] = "";
    assert(ndpi_strlcpy(dst_empty, "", sizeof(dst_empty), 0) == 0);
    assert(dst_empty[0] == '\0');

    // Basic copy test
    char dst1[10] = "";
    assert(ndpi_strlcpy(dst1, "abc", sizeof(dst1), 3) == 3);
    assert(strcmp(dst1, "abc") == 0);

    // Test with dst_len smaller than src_len
    char dst2[4] = "";
    assert(ndpi_strlcpy(dst2, "abcdef", sizeof(dst2), 6) == 6);
    assert(strcmp(dst2, "abc") == 0); // Should truncate "abcdef" to "abc"

    // Test with dst_len bigger than src_len
    char dst3[10] = "";
    assert(ndpi_strlcpy(dst3, "abc", sizeof(dst3), 3) == 3);
    assert(strcmp(dst3, "abc") == 0);

    // Test with dst_len equal to 1 (only null terminator should be copied)
    char dst4[1];
    assert(ndpi_strlcpy(dst4, "abc", sizeof(dst4), 3) == 3);
    assert(dst4[0] == '\0'); // Should only contain the null terminator

    // Test with NULL source, expecting return value to be 0
    char dst5[10];
    assert(ndpi_strlcpy(dst5, NULL, sizeof(dst5), 0) == 0);

    // Test with NULL destination, should also return 0 without crashing
    assert(ndpi_strlcpy(NULL, "abc", sizeof(dst5), 3) == 0);
}

/* *********************************************** */

void strnstrUnitTest(void) {
    /* Test 1: null string */
    assert(ndpi_strnstr(NULL, "find", 10) == NULL);
    assert(ndpi_strnstr("string", NULL, 10) == NULL);

    /* Test 2: empty substring */
    assert(strcmp(ndpi_strnstr("string", "", 6), "string") == 0);

    /* Test 3: single character substring */
    assert(strcmp(ndpi_strnstr("string", "r", 6), "ring") == 0);
    assert(ndpi_strnstr("string", "x", 6) == NULL);

    /* Test 4: multiple character substring */
    assert(strcmp(ndpi_strnstr("string", "ing", 6), "ing") == 0);
    assert(ndpi_strnstr("string", "xyz", 6) == NULL);

    /* Test 5: substring equal to the beginning of the string */
    assert(strcmp(ndpi_strnstr("string", "str", 3), "string") == 0);

    /* Test 6: substring at the end of the string */
    assert(strcmp(ndpi_strnstr("string", "ing", 6), "ing") == 0);

    /* Test 7: substring in the middle of the string */
    assert(strcmp(ndpi_strnstr("hello world", "lo wo", 11), "lo world") == 0);

    /* Test 8: repeated characters in the string */
    assert(strcmp(ndpi_strnstr("aaaaaa", "aaa", 6), "aaaaaa") == 0);

    /* Test 9: empty string and slen 0 */
    assert(ndpi_strnstr("", "find", 0) == NULL);

    /* Test 10: substring equal to the string */
    assert(strcmp(ndpi_strnstr("string", "string", 6), "string") == 0);

    /* Test 11a,b: max_length bigger that string length */
    assert(strcmp(ndpi_strnstr("string", "string", 66), "string") == 0);
    assert(ndpi_strnstr("string", "a", 66) == NULL);

    /* Test 12: substring longer than the string */
    assert(ndpi_strnstr("string", "stringA", 6) == NULL);

    /* Test 13 */
    assert(ndpi_strnstr("abcdef", "abc", 2) == NULL);

    /* Test 14: zero length */
    assert(strcmp(ndpi_strnstr("", "", 0), "") == 0);
    assert(strcmp(ndpi_strnstr("string", "", 0), "string") == 0);
    assert(ndpi_strnstr("", "str", 0) == NULL);
    assert(ndpi_strnstr("string", "str", 0) == NULL);
    assert(ndpi_strnstr("str", "string", 0) == NULL);
}

/* *********************************************** */

void strncasestrUnitTest(void) {
    /* Test 1: null string */
    assert(ndpi_strncasestr(NULL, "find", 10) == NULL);
    assert(ndpi_strncasestr("string", NULL, 10) == NULL);

    /* Test 2: empty substring */
    assert(strcmp(ndpi_strncasestr("string", "", 6), "string") == 0);

    /* Test 3: single character substring */
    assert(strcmp(ndpi_strncasestr("string", "r", 6), "ring") == 0);
    assert(strcmp(ndpi_strncasestr("string", "R", 6), "ring") == 0);
    assert(strcmp(ndpi_strncasestr("stRing", "r", 6), "Ring") == 0);
    assert(ndpi_strncasestr("string", "x", 6) == NULL);
    assert(ndpi_strncasestr("string", "X", 6) == NULL);

    /* Test 4: multiple character substring */
    assert(strcmp(ndpi_strncasestr("string", "ing", 6), "ing") == 0);
    assert(strcmp(ndpi_strncasestr("striNg", "InG", 6), "iNg") == 0);
    assert(ndpi_strncasestr("string", "xyz", 6) == NULL);
    assert(ndpi_strncasestr("striNg", "XyZ", 6) == NULL);

    /* Test 5: substring equal to the beginning of the string */
    assert(strcmp(ndpi_strncasestr("string", "str", 5), "string") == 0);
    assert(strcmp(ndpi_strncasestr("string", "sTR", 5), "string") == 0);
    assert(strcmp(ndpi_strncasestr("String", "STR", 5), "String") == 0);
    assert(strcmp(ndpi_strncasestr("Long Long String", "long long", 15), "Long Long String") == 0);

    /* Test 6: substring at the end of the string */
    assert(strcmp(ndpi_strncasestr("string", "ing", 6), "ing") == 0);
    assert(strcmp(ndpi_strncasestr("some longer STRing", "GEr sTrING", 18), "ger STRing") == 0);

    /* Test 7: substring in the middle of the string */
    assert(strcmp(ndpi_strncasestr("hello world", "lo wo", 11), "lo world") == 0);
    assert(strcmp(ndpi_strncasestr("hello BEAUTIFUL world", "beautiful", 20), "BEAUTIFUL world") == 0);

    /* Test 8: repeated characters in the string */
    assert(strcmp(ndpi_strncasestr("aaaaaa", "aaa", 6), "aaaaaa") == 0);
    assert(strcmp(ndpi_strncasestr("aaAaAa", "aaa", 6), "aaAaAa") == 0);
    assert(strcmp(ndpi_strncasestr("AAAaaa", "aaa", 6), "AAAaaa") == 0);

    /* Test 9: empty string and slen 0 */
    assert(ndpi_strncasestr("", "find", 0) == NULL);

    /* Test 10: substring equal to the string */
    assert(strcmp(ndpi_strncasestr("string", "string", 6), "string") == 0);
    assert(strcmp(ndpi_strncasestr("string", "STRING", 6), "string") == 0);
    assert(strcmp(ndpi_strncasestr("sTrInG", "StRiNg", 6), "sTrInG") == 0);

    /* Test 11a,b: max_length bigger that string length */
    assert(strcmp(ndpi_strncasestr("string", "string", 66), "string") == 0);
    assert(ndpi_strncasestr("string", "a", 66) == NULL);

    /* Test 12: substring longer than the string */
    assert(ndpi_strncasestr("string", "stringA", 6) == NULL);

    /* Test 13 */
    assert(ndpi_strncasestr("abcdef", "abc", 2) == NULL);

    /* Test 14: zero length */
    assert(strcmp(ndpi_strncasestr("", "", 0), "") == 0);
    assert(strcmp(ndpi_strncasestr("string", "", 0), "string") == 0);
    assert(ndpi_strncasestr("", "str", 0) == NULL);
    assert(ndpi_strncasestr("string", "str", 0) == NULL);
    assert(ndpi_strncasestr("str", "string", 0) == NULL);
}

/* *********************************************** */

void memmemUnitTest(void) {
    /* Test 1: null string */
    assert(ndpi_memmem(NULL, 0, NULL, 0) == NULL);
    assert(ndpi_memmem(NULL, 0, NULL, 10) == NULL);
    assert(ndpi_memmem(NULL, 0, "find", 10) == NULL);
    assert(ndpi_memmem(NULL, 10, "find", 10) == NULL);
    assert(ndpi_memmem("string", 10, NULL, 0) == NULL);
    assert(ndpi_memmem("string", 10, NULL, 10) == NULL);

    /* Test 2: zero length */
    assert(strcmp(ndpi_memmem("", 0, "", 0), "") == 0);
    assert(strcmp(ndpi_memmem("string", 6, "", 0), "string") == 0);
    assert(strcmp(ndpi_memmem("string", 0, "", 0), "string") == 0);
    assert(ndpi_memmem("", 0, "string", 6) == NULL);

    /* Test 3: empty substring */
    assert(strcmp(ndpi_memmem("string", 6, "", 0), "string") == 0);

    /* Test 4: single character substring */
    assert(strcmp(ndpi_memmem("string", 6, "r", 1), "ring") == 0);
    assert(ndpi_memmem("string", 6, "x", 1) == NULL);

    /* Test 5: multiple character substring */
    assert(strcmp(ndpi_memmem("string", 6, "ing", 3), "ing") == 0);
    assert(ndpi_memmem("string", 6, "xyz", 3) == NULL);

    /* Test 6: substring equal to the beginning of the string */
    assert(strcmp(ndpi_memmem("string", 6, "str", 3), "string") == 0);

    /* Test 7: substring at the end of the string */
    assert(strcmp(ndpi_memmem("string", 6, "ing", 3), "ing") == 0);

    /* Test 8: substring in the middle of the string */
    assert(strcmp(ndpi_memmem("hello world", strlen("hello world"), "lo wo", strlen("lo wo")), "lo world") == 0);

    /* Test 9: repeated characters in the string */
    assert(strcmp(ndpi_memmem("aaaaaa", 6, "aaa", 3), "aaaaaa") == 0);

    /* Test 10: substring equal to the string */
    assert(strcmp(ndpi_memmem("string", 6, "string", 6), "string") == 0);

    /* Test 11: substring longer than the string */
    assert(ndpi_memmem("string", 6, "stringA", 7) == NULL);
}

/* *********************************************** */

void mahalanobisUnitTest()
{
    /* Example based on: https://supplychenmanagement.com/2019/03/06/calculating-mahalanobis-distance/ */

    const float i_s[3 * 3] = { 0.0482486100061447, -0.00420645518018837, -0.0138921893248235,
                    -0.00420645518018836, 0.00177288408892603, -0.00649813703331057,
                    -0.0138921893248235, -0.00649813703331056,  0.066800436339011 }; /* Inverted covar matrix */
    const float u[3] = { 22.8, 180.0, 9.2 }; /* Means vector */
    u_int32_t x[3] = { 26, 167, 12 }; /* Point */
    float md;

    md = ndpi_mahalanobis_distance(x, 3, u, i_s);
    /* It is a bit tricky to test float equality on different archs -> loose check.
     * md sholud be 1.3753 */
    assert(md >= 1.37 && md <= 1.38);
}

/* *********************************************** */

void filterUnitTest() {
    ndpi_filter* f = ndpi_filter_alloc();
    u_int32_t v, i;

    assert(f);

    srand(time(NULL));

    for (i = 0; i < 1000; i++)
        assert(ndpi_filter_add(f, v = rand()));

    assert(ndpi_filter_contains(f, v));

    ndpi_filter_free(f);
}

/* *********************************************** */

void zscoreUnitTest() {
    u_int32_t values[] = { 1, 3, 3, 4, 5, 2, 6, 7, 30, 16 };
    u_int32_t i;
    u_int32_t num_outliers;
    u_int32_t const num = NDPI_ARRAY_LENGTH(values);
    bool outliers[NDPI_ARRAY_LENGTH(values)], do_trace = false;

    num_outliers = ndpi_find_outliers(values, outliers, num);

    if (do_trace) {
        printf("outliers: %u\n", num_outliers);

        for (i = 0; i < num; i++)
            printf("%u %s\n", values[i], outliers[i] ? "OUTLIER" : "OK");
    }
}

/* *********************************************** */

void linearUnitTest() {
    u_int32_t values[] = { 15, 27, 38, 49, 68, 72, 90, 150, 175, 203 };
    u_int32_t prediction;
    u_int32_t const num = NDPI_ARRAY_LENGTH(values);
    bool do_trace = false;
    int rc = ndpi_predict_linear(values, num, 2 * num, &prediction);

    if (do_trace) {
        printf("[rc: %d][predicted value: %u]\n", rc, prediction);
    }
}

/* *********************************************** */

void sketchUnitTest() {
    struct ndpi_cm_sketch* sketch;

#if 0
    ndpi_cm_sketch_init(8);
    ndpi_cm_sketch_init(16);
    ndpi_cm_sketch_init(32);
    ndpi_cm_sketch_init(64);
    ndpi_cm_sketch_init(256);
    ndpi_cm_sketch_init(512);
    ndpi_cm_sketch_init(1024);
    ndpi_cm_sketch_init(2048);
    ndpi_cm_sketch_init(4096);
    ndpi_cm_sketch_init(8192);
    exit(0);
#endif

    sketch = ndpi_cm_sketch_init(32);

    if (sketch) {
        u_int32_t i, num_one = 0;
        bool do_trace = false;

        srand(time(NULL));

        for (i = 0; i < 10000; i++) {
            u_int32_t v = rand() % 1000;

            if (v == 1) num_one++;
            ndpi_cm_sketch_add(sketch, v);
        }

        if (do_trace)
            printf("The estimated count of 1 is %u [expectedl: %u]\n",
                ndpi_cm_sketch_count(sketch, 1), num_one);

        ndpi_cm_sketch_destroy(sketch);

        if (do_trace)
            exit(0);
    }
}

/* *********************************************** */

void binaryBitmapUnitTest() {
    ndpi_binary_bitmap* b = ndpi_binary_bitmap_alloc();
    u_int64_t hashval = 8149764909040470312;
    u_int8_t category = 33;

    ndpi_binary_bitmap_set(b, hashval, category);
    ndpi_binary_bitmap_set(b, hashval + 1, category);
    category = 0;
    assert(ndpi_binary_bitmap_isset(b, hashval, &category));
    assert(category == 33);
    ndpi_binary_bitmap_free(b);
}

/* *********************************************** */

void pearsonUnitTest() {
    u_int32_t data_a[] = { 1, 2, 3, 4, 5 };
    u_int32_t data_b[] = { 1000, 113, 104, 105, 106 };
    u_int16_t num = sizeof(data_a) / sizeof(u_int32_t);
    float pearson = ndpi_pearson_correlation(data_a, data_b, num);

    assert(pearson != 0.0);
    // printf("%.8f\n", pearson);
}

/* *********************************************** */

void outlierUnitTest() {
    u_int32_t data[] = { 1, 2, 3, 4, 5 };
    u_int16_t num = sizeof(data) / sizeof(u_int32_t);
    u_int16_t value_to_check = 8;
    float threshold = 1.5, lower, upper;
    float is_outlier = ndpi_is_outlier(data, num, value_to_check,
        threshold, &lower, &upper);

    /* printf("%.2f < %u < %.2f : %s\n", lower, value_to_check, upper, is_outlier ? "OUTLIER" : "OK"); */
    assert(is_outlier == true);
}

/* *********************************************** */

void loadStressTest() {
    struct ndpi_detection_module_struct* ndpi_struct_shadow = ndpi_init_detection_module(NULL);
    NDPI_PROTOCOL_BITMASK all;

    if (ndpi_struct_shadow) {
        int i;

        NDPI_BITMASK_SET_ALL(all);
        ndpi_set_protocol_detection_bitmask2(ndpi_struct_shadow, &all);

        for (i = 1; i < 100000; i++) {
            char name[32];
            ndpi_protocol_category_t id = CUSTOM_CATEGORY_MALWARE;
            u_int8_t value = (u_int8_t)i;

            snprintf(name, sizeof(name), "%d.com", i);
            ndpi_load_hostname_category(ndpi_struct_shadow, name, id);

            snprintf(name, sizeof(name), "%u.%u.%u.%u", value, value, value, value);
            ndpi_load_ip_category(ndpi_struct_shadow, name, id, (void*)"My list");
        }

        ndpi_enable_loaded_categories(ndpi_struct_shadow);
        ndpi_finalize_initialization(ndpi_struct_shadow);
        ndpi_exit_detection_module(ndpi_struct_shadow);
    }
}

/* *********************************************** */

void kdUnitTest() {
    ndpi_kd_tree* t = ndpi_kd_create(5);
    double v[][5] = {
      { 0, 4, 2, 3, 4 },
      { 0, 1, 2, 3, 6 },
      { 1, 2, 3, 4, 5 },
    };
    double v1[5] = { 0, 1, 2, 3, 8 };
    u_int i, sz = 5 * sizeof(double), num = sizeof(v) / sz;
    ndpi_kd_tree_result* res;
    double* ret, * to_find = v[1];

    assert(t);

    for (i = 0; i < num; i++)
        assert(ndpi_kd_insert(t, v[i], NULL) == true);

    assert((res = ndpi_kd_nearest(t, to_find)) != NULL);
    assert(ndpi_kd_num_results(res) == 1);
    assert((ret = ndpi_kd_result_get_item(res, NULL)) != NULL);
    assert(memcmp(ret, to_find, sz) == 0);
    ndpi_kd_result_free(res);

    assert((res = ndpi_kd_nearest(t, v1)) != NULL);
    assert(ndpi_kd_num_results(res) == 1);
    assert((ret = ndpi_kd_result_get_item(res, NULL)) != NULL);
    assert(memcmp(ret, v1, sz) != 0);
    assert(ndpi_kd_distance(ret, v1, 5) == 4.);
    ndpi_kd_result_free(res);

    ndpi_kd_free(t);
}

/* *********************************************** */

void ballTreeUnitTest() {
    ndpi_btree* ball_tree;
    double v[][5] = {
      { 0, 4, 2, 3, 4 },
      { 0, 1, 2, 3, 6 },
      { 1, 2, 3, 4, 5 },
    };
    double v1[] = { 0, 1, 2, 3, 8 };
    double* rows[] = { v[0], v[1], v[2] };
    double* q_rows[] = { v1 };
    u_int32_t num_columns = 5;
    u_int32_t num_rows = sizeof(v) / (sizeof(double) * num_columns);
    ndpi_knn result;
    u_int32_t nun_results = 2;
    int i, j;

    ball_tree = ndpi_btree_init(rows, num_rows, num_columns);
    assert(ball_tree != NULL);
    result = ndpi_btree_query(ball_tree, q_rows,
        sizeof(q_rows) / sizeof(double*),
        num_columns, nun_results);

    assert(result.n_samples == 2);

    for (i = 0; i < result.n_samples; i++) {
        printf("{\"knn_idx\": [");
        for (j = 0; j < result.n_neighbors; j++)
        {
            printf("%d", result.indices[i][j]);
            if (j != result.n_neighbors - 1)
                printf(", ");
        }
        printf("],\n \"knn_dist\": [");
        for (j = 0; j < result.n_neighbors; j++)
        {
            printf("%.12lf", result.distances[i][j]);
            if (j != result.n_neighbors - 1)
                printf(", ");
        }
        printf("]\n}\n");
        if (i != result.n_samples - 1)
            printf(", ");
    }

    ndpi_free_knn(result);
    ndpi_free_btree(ball_tree);
}

/* *********************************************** */

void cryptDecryptUnitTest() {
    u_char enc_dec_key[64] = "9dedb817e5a8805c1de62eb8982665b9a2b4715174c34d23b9a46ffafacfb2a7" /* SHA256("nDPI") */;
    const char* test_string = "The quick brown fox jumps over the lazy dog";
    char* enc, * dec;
    u_int16_t e_len, d_len, t_len = strlen(test_string);

    enc = ndpi_quick_encrypt(test_string, t_len, &e_len, enc_dec_key);
    assert(enc != NULL);
    dec = ndpi_quick_decrypt((const char*)enc, e_len, &d_len, enc_dec_key);
    assert(dec != NULL);
    assert(t_len == d_len);

    assert(strncmp(dec, test_string, e_len) == 0);

    ndpi_free(enc);
    ndpi_free(dec);
}

/* *********************************************** */

void encodeDomainsUnitTest() {
    NDPI_PROTOCOL_BITMASK all;
    struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);
    const char* lists_path = "../lists/public_suffix_list.dat";
    struct stat st;

    if (stat(lists_path, &st) == 0) {
        u_int16_t suffix_id;
        char out[256];
        char* str;
        ndpi_protocol_category_t id;

        NDPI_BITMASK_SET_ALL(all);
        ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);

        assert(ndpi_load_domain_suffixes(ndpi_str, (char*)lists_path) == 0);

        ndpi_get_host_domain_suffix(ndpi_str, "lcb.it", &suffix_id);
        ndpi_get_host_domain_suffix(ndpi_str, "www.ntop.org", &suffix_id);
        ndpi_get_host_domain_suffix(ndpi_str, "www.bbc.co.uk", &suffix_id);

        str = (char*)"www.ntop.org"; assert(ndpi_encode_domain(ndpi_str, str, out, sizeof(out)) == 8);
        str = (char*)"www.bbc.co.uk"; assert(ndpi_encode_domain(ndpi_str, str, out, sizeof(out)) == 8);

        assert(ndpi_load_categories_dir(ndpi_str, "../lists"));
        assert(ndpi_load_categories_file(ndpi_str, "./categories.txt", "categories.txt"));

        str = (char*)"2001:db8:1::1"; assert(ndpi_get_custom_category_match(ndpi_str, str, strlen(str), &id) == 0); assert(id == 100);
        str = (char*)"www.internetbadguys.com"; assert(ndpi_get_custom_category_match(ndpi_str, str, strlen(str), &id) == 0); assert(id == 100);
        str = (char*)"0grand-casino.com"; assert(ndpi_get_custom_category_match(ndpi_str, str, strlen(str), &id) == 0); assert(id == 107);
        str = (char*)"222.0grand-casino.com"; assert(ndpi_get_custom_category_match(ndpi_str, str, strlen(str), &id) == 0); assert(id == 107);
        str = (char*)"10bet.com"; assert(ndpi_get_custom_category_match(ndpi_str, str, strlen(str), &id) == 0); assert(id == 107);
        str = (char*)"www.ntop.org"; assert(ndpi_get_custom_category_match(ndpi_str, str, strlen(str), &id) == -1); assert(id == 0);
        str = (char*)"www.andrewpope.com"; assert(ndpi_get_custom_category_match(ndpi_str, str, strlen(str), &id) == 0); assert(id == 100);
    }

    ndpi_exit_detection_module(ndpi_str);
}

/* *********************************************** */

void domainsUnitTest() {
    NDPI_PROTOCOL_BITMASK all;
    struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);
    const char* lists_path = "../lists/public_suffix_list.dat";
    struct stat st;

    if (stat(lists_path, &st) == 0) {
        u_int16_t suffix_id;

        NDPI_BITMASK_SET_ALL(all);
        ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);

        assert(ndpi_load_domain_suffixes(ndpi_str, (char*)lists_path) == 0);

        assert(strcmp(ndpi_get_host_domain(ndpi_str, "1.0.0.127.in-addr.arpa"), "in-addr.arpa") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "fe80::fd:5447:b2d1:40e0"), "fe80::fd:5447:b2d1:40e0") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "192.168.1.2"), "192.168.1.2") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "extension.femetrics.grammarly.io"), "grammarly.io") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "www.ovh.commander1.com"), "commander1.com") == 0);

        assert(strcmp(ndpi_get_host_domain_suffix(ndpi_str, "www.chosei.chiba.jp", &suffix_id), "chosei.chiba.jp") == 0);
        assert(strcmp(ndpi_get_host_domain_suffix(ndpi_str, "www.unipi.it", &suffix_id), "it") == 0);
        assert(strcmp(ndpi_get_host_domain_suffix(ndpi_str, "mail.apple.com", &suffix_id), "com") == 0);
        assert(strcmp(ndpi_get_host_domain_suffix(ndpi_str, "www.bbc.co.uk", &suffix_id), "co.uk") == 0);

        assert(strcmp(ndpi_get_host_domain(ndpi_str, "www.chosei.chiba.jp"), "www.chosei.chiba.jp") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "www.unipi.it"), "unipi.it") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "mail.apple.com"), "apple.com") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "www.bbc.co.uk"), "bbc.co.uk") == 0);
        assert(strcmp(ndpi_get_host_domain(ndpi_str, "zy1ssnfwwl.execute-api.eu-north-1.amazonaws.com"), "amazonaws.com") == 0);
    }

    ndpi_exit_detection_module(ndpi_str);
}

/* *********************************************** */

void domainSearchUnitTest() {
    ndpi_domain_classify* sc = ndpi_domain_classify_alloc();
    char* domain = "ntop.org";
    u_int16_t class_id;
    struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);
    u_int8_t trace = 0;
    NDPI_PROTOCOL_BITMASK all;

    assert(ndpi_str);
    assert(sc);

    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);
    ndpi_finalize_initialization(ndpi_str);

    ndpi_domain_classify_add(ndpi_str, sc, NDPI_PROTOCOL_NTOP, ".ntop.org");
    ndpi_domain_classify_add(ndpi_str, sc, NDPI_PROTOCOL_NTOP, domain);
    assert(ndpi_domain_classify_hostname(ndpi_str, sc, &class_id, domain));
    assert(class_id == NDPI_PROTOCOL_NTOP);

    ndpi_domain_classify_add(ndpi_str, sc, NDPI_PROTOCOL_CATEGORY_GAMBLING, "123vc.club");
    assert(ndpi_domain_classify_hostname(ndpi_str, sc, &class_id, "123vc.club"));
    assert(class_id == NDPI_PROTOCOL_CATEGORY_GAMBLING);

    /* Subdomain check */
    assert(ndpi_domain_classify_hostname(ndpi_str, sc, &class_id, "blog.ntop.org"));
    assert(class_id == NDPI_PROTOCOL_NTOP);

    u_int32_t s = ndpi_domain_classify_size(sc);
    if (trace) printf("ndpi_domain_classify size: %u \n", s);


    ndpi_domain_classify_free(sc);
    ndpi_exit_detection_module(ndpi_str);
}

/* *********************************************** */

void domainSearchUnitTest2() {
    struct ndpi_detection_module_struct* ndpi_str = ndpi_init_detection_module(NULL);
    ndpi_domain_classify* c = ndpi_domain_classify_alloc();
    u_int16_t class_id = 9;
    NDPI_PROTOCOL_BITMASK all;

    assert(ndpi_str);
    assert(c);

    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_str, &all);
    ndpi_finalize_initialization(ndpi_str);

    ndpi_domain_classify_add(ndpi_str, c, class_id, "ntop.org");
    ndpi_domain_classify_add(ndpi_str, c, class_id, "apple.com");

    assert(!ndpi_domain_classify_hostname(ndpi_str, c, &class_id, "ntop.com"));

    ndpi_domain_classify_free(c);
    ndpi_exit_detection_module(ndpi_str);
}

/* *********************************************** */

void domainCacheTestUnit() {
    struct ndpi_address_cache* cache = ndpi_init_address_cache(32000);
    ndpi_ip_addr_t ip;
    u_int32_t epoch_now = (u_int32_t)time(NULL);
    struct ndpi_address_cache_item* ret;
    char fname[64] = { 0 };

    assert(cache);

    /* On GitHub Actions, ndpiReader might be called multiple times in parallel, so
       every instance must use its own file */
    snprintf(fname, sizeof(fname), "./cache.%u.dump", (unsigned int)getpid());

    memset(&ip, 0, sizeof(ip));
    ip.ipv4 = 12345678;
    assert(ndpi_address_cache_insert(cache, ip, "nodomain.local", epoch_now, 32) == true);

    ip.ipv4 = 87654321;
    assert(ndpi_address_cache_insert(cache, ip, "hello.local", epoch_now, 0) == true);

    assert((ret = ndpi_address_cache_find(cache, ip, epoch_now)) != NULL);
    assert(strcmp(ret->hostname, "hello.local") == 0);
    assert(ndpi_address_cache_find(cache, ip, epoch_now + 1) == NULL);

    assert(ndpi_address_cache_dump(cache, fname, epoch_now));
    ndpi_term_address_cache(cache);

    cache = ndpi_init_address_cache(32000);
    assert(cache);
    assert(ndpi_address_cache_restore(cache, fname, epoch_now) == 1);

    ip.ipv4 = 12345678;
    assert((ret = ndpi_address_cache_find(cache, ip, epoch_now)) != NULL);
    assert(strcmp(ret->hostname, "nodomain.local") == 0);

    ndpi_term_address_cache(cache);
    unlink(fname);
}


void run_unittests() {

    domainCacheTestUnit();
    cryptDecryptUnitTest();
    kdUnitTest();
    encodeDomainsUnitTest();
    loadStressTest();
    domainsUnitTest();
    outlierUnitTest();
    pearsonUnitTest();
    binaryBitmapUnitTest();
    domainSearchUnitTest();
    domainSearchUnitTest2();
    sketchUnitTest();
    linearUnitTest();
    zscoreUnitTest();
    sesUnitTest();
    desUnitTest();

    /* Internal checks */
    // binUnitTest();
    //hwUnitTest();
    jitterUnitTest();
    rsiUnitTest();
    hashUnitTest();
    dgaUnitTest();
    hllUnitTest();
    bitmapUnitTest();
    filterUnitTest();
    automataUnitTest();
    automataDomainsUnitTest();
    analyzeUnitTest();
    ndpi_self_check_host_match(stderr);
    analysisUnitTest();
    compressedBitmapUnitTest();
    strtonumUnitTest();
    strlcpyUnitTest();
    strnstrUnitTest();
    strncasestrUnitTest();
    memmemUnitTest();
    mahalanobisUnitTest();

}