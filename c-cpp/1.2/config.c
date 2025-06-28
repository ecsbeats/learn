#include <assert.h>
#include <stdio.h>

static const char config_data[] = {
#embed "config.dat"
};

void setup_environment() {
  assert(sizeof(config_data) > 0);
}

void test_parse_config_with_warning() {
  parse_config(&config_data, sizeof(config_data));
}

void test_parse_config() {
  bool result = parse_config(&config_data, sizeof(config_data));
  assert(result == true);
}

void test_parse_config_v1() {
  bool result = parse_config_v1(); // should return deprecated error
  assert(result == true);
}

int main() {
  setup_environment();
  test_parse_config_with_warning(); // Should emit nodiscard warning
  test_parse_config();
  test_parse_config_v1(); // Should emit deprecated warning and point users to the `parse_config` function
}
