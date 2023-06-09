#define SECRET
#define THINGNAME "THING NAME"

const char WIFI_SSID[] = "SSID";
const char WIFI_PASSWORD[] = "PASSWORD";
const char AWS_IOT_ENDPOINT[] = "xxx.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";
