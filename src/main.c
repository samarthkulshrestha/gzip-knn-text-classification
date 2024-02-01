#include <stdio.h>
#include <zlib.h>

#define NOB_IMPLEMENTATION
#include "oldnob.h"

#define UNUSED(x) (void)(x)

Nob_String_View deflate_sv(Nob_String_View sv) {
    void *output = nob_temp_alloc(sv.count * 2);

    z_stream defstream = {0};
    defstream.avail_in = (uInt)sv.count;
    defstream.next_in = (Bytef *)sv.data;
    defstream.avail_out = (uInt)sv.count*2;
    defstream.next_out = (Bytef *)output;

    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    return nob_sv_from_parts(output, defstream.total_out);
}

typedef struct {
    size_t class;
    Nob_String_View text;
} Sample;

typedef struct {
    Sample *items;
    size_t capacity;
    size_t count;
} Samples;

Samples parse_samples(Nob_String_View content) {
    Samples samples = {0};
    size_t line_count = 0;

    for (; content.count > 0; ++line_count) {
        Nob_String_View line = nob_sv_chop_by_delim(&content, '\n');

        if (line_count == 0) continue;       // ignore header

        Nob_String_View class = nob_sv_chop_by_delim(&line, ',');
        size_t class_index = *class.data - '0' - 1;

        nob_da_append(&samples, ((Sample){
                .class = class_index,
                .text = line,
                }));
    }

    return samples;
}

const char *class_names[] = {"World", "Sports", "Business", "Sci/Tech"};

typedef struct {
    float distance;
    size_t class;
} NCD;

typedef struct {
    NCD *items;
    size_t count;
    size_t capacity;
} NCDs;

float ncd(Nob_String_View a, Nob_String_View b, float cb) {
    Nob_String_View ab = nob_sv_from_cstr(nob_temp_sprintf(SV_Fmt SV_Fmt, SV_Arg(a), SV_Arg(b)));

    float ca = deflate_sv(a).count;
    float cab = deflate_sv(ab).count;
    float mn = ca; if (mn > cb) mn = cb;
    float mx = ca; if (mx < cb) mx = cb;
    return (cab - mn) / mx;
}

int compar_ncds(const void *a, const void *b) {
    const NCD *na = a;
    const NCD *nb = b;
    if (na->distance < nb->distance) return -1;
    if (na->distance > nb->distance) return 1;
    return 0;
}

size_t classify_sample(Samples train, Nob_String_View text, size_t k) {
    NCDs ncds = {0};

    float cb = deflate_sv(text).count;
    for (size_t i = 0; i < train.count; ++i) {
        float distance = ncd(train.items[i].text, text, cb);
        nob_temp_reset();
        nob_da_append(&ncds, ((NCD) {
                    .distance = distance,
                    .class = train.items[i].class,
                    }));
        printf("\rclassifying %zu/%zu", i, train.count);
    }
    printf("\n");

    qsort(ncds.items, ncds.count, sizeof(*ncds.items), compar_ncds);
    size_t class_freq[NOB_ARRAY_LEN(class_names)] = {0};

    for (size_t i = 0; i < ncds.count && i < k; ++i) {
        class_freq[ncds.items[i].class] += 1;
    }

    size_t predicted_class = 0;
    for (size_t i = 1; i < NOB_ARRAY_LEN(class_names); ++i) {
        if (class_freq[predicted_class] < class_freq[i]) {
            predicted_class = i;
        }
    }

    return predicted_class;
}

int main(int argc, char **argv) {
    const char *program = nob_shift_args(&argc, &argv);

    if (argc <= 0) {
        nob_log(NOB_ERROR, "usage: %s <train.csv> <test.csv>", program);
        nob_log(NOB_ERROR, "training file not provided.");
        return(1);
    }

    const char *train_path = nob_shift_args(&argc, &argv);
    Nob_String_Builder train_content = {0};
    if (!nob_read_entire_file(train_path, &train_content)) return 1;
    Samples train_samples = parse_samples(nob_sv_from_parts(train_content.items, train_content.count));

    if (argc <= 0) {
        nob_log(NOB_ERROR, "usage: %s <train.csv> <test.csv>", program);
        nob_log(NOB_ERROR, "testing file not provided.");
        return(1);
    }

    const char *test_path = nob_shift_args(&argc, &argv);
    Nob_String_Builder test_content = {0};
    if (!nob_read_entire_file(test_path, &test_content)) return 1;
    Samples test_samples = parse_samples(nob_sv_from_parts(test_content.items, test_content.count));
    UNUSED(test_samples);

    // const char *text = "Attacks on ships in the Red Sea are disrupting global trade. Here’s how it could affect what you buy. Attacks on ships in the Red Sea by Yemen’s Houthi rebels have unraveled a key global trade route, forcing vessels into longer and more costly journeys around Africa.";
    const char *text = "Sinner rallies from 2 sets down to beat Medvedev in Australia and clinch his first Grand Slam title. Jannik Sinner has rallied from two sets down to win the Australian Open final against Daniil Medvedev and clinch his first Grand Slam title.";

    size_t class = classify_sample(train_samples, nob_sv_from_cstr(text), 5);
    nob_log(NOB_INFO, "text: %s", text);
    nob_log(NOB_INFO, "class: %s", class_names[class]);

    // Sample sample = test_samples.items[1];
    // size_t predicted_class = classify_sample(train_samples, sample.text, 5);
    //
    // nob_log(NOB_INFO, "text: "SV_Fmt, SV_Arg(sample.text));
    // nob_log(NOB_INFO, "predicted class: %s", class_names[predicted_class]);
    // nob_log(NOB_INFO, "actual class: %s", class_names[sample.class]);

    return 0;
}
