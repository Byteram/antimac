/*
 * antimac.c
 * A MAC address management tool for macOS (C version)
 * Authors: @vvrmatos (aka @spacemany2k38)
 * Date: 2025-07-18
 *
 * Features:
 *   - Generate and set a random MAC address:      antimac <device>
 *   - Show the current MAC address:               antimac -s <device> | --show <device>
 *   - Set a specific MAC address:                 antimac -c <device> <new-mac-address> | --config <device> <new-mac-address>
 *   - Show version:                               antimac -v | --version
 *
 *  MacOSX only, requires root privileges.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>

#define VERSION "0.0.1"

void print_usage() {
    printf("Usage:\n");
    printf("  antimac <device>\n");
    printf("      Generate and set a random MAC address for <device>\n");
    printf("  antimac -s <device>\n");
    printf("  antimac --show <device>\n");
    printf("      Show the current MAC address for <device>\n");
    printf("  antimac -c <device> <new-mac-address>\n");
    printf("  antimac --config <device> <new-mac-address>\n");
    printf("      Set <new-mac-address> for <device>\n");
    printf("  antimac -v\n");
    printf("  antimac --version\n");
    printf("      Show version\n");
}

void print_version() {
    printf("Version: %s\n", VERSION);
}

void run_command(const char *cmd, char *output, size_t outlen) {
    FILE *fp = popen(cmd, "r");
    if (fp) {
        if (fgets(output, (int)outlen, fp) == NULL) {
            output[0] = '\0';
        }
        pclose(fp);
    } else {
        output[0] = '\0';
    }
}

void show_mac(const char *device) {
    char cmd[256], buf[256];
    snprintf(cmd, sizeof(cmd), "/sbin/ifconfig %s | grep ether", device);
    run_command(cmd, buf, sizeof(buf));
    char *mac = strstr(buf, "ether ");
    if (mac) {
        mac += 6;
        char *end = strchr(mac, '\n');
        if (end) *end = '\0';
        printf("[+] %s MAC address: %s\n", device, mac);
    } else {
        printf("Could not get MAC address for %s\n", device);
    }
}

void generate_mac(char *macbuf) {
    unsigned char mac[6];
    srand((unsigned int)time(NULL) ^ getpid());
    mac[0] = (rand() & 0xFE) | 0x02;
    for (int i = 1; i < 6; ++i) mac[i] = rand() & 0xFF;
    snprintf(macbuf, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool valid_mac(const char *mac) {
    if (!mac) return false;
    int n = 0;
    for (int i = 0; mac[i]; ++i) {
        if ((i+1) % 3 == 0) {
            if (mac[i] != ':' && mac[i+1] != '\0') return false;
            ++n;
        } else if (!((mac[i] >= '0' && mac[i] <= '9') || (mac[i] >= 'a' && mac[i] <= 'f') || (mac[i] >= 'A' && mac[i] <= 'F'))) {
            return false;
        }
    }
    return n == 5 && strlen(mac) == 17;
}

bool device_down(const char *device) {
    char cmd[256], buf[256];
    snprintf(cmd, sizeof(cmd), "/sbin/ifconfig -d | grep -E '^%s:'", device);
    run_command(cmd, buf, sizeof(buf));
    return buf[0] != '\0';
}

void get_cpu_arch(char *arch, size_t len) {
    run_command("uname -m", arch, len);
    size_t l = strlen(arch);
    if (l > 0 && arch[l-1] == '\n') arch[l-1] = '\0';
}

void get_interface_type(const char *device, char *type, size_t len) {
    char cmd[256], buf[256];
    snprintf(cmd, sizeof(cmd), "networksetup -listallhardwareports | grep -A1 'Wi-Fi' | grep '%s'", device);
    run_command(cmd, buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(type, "Wi-Fi", len);
        return;
    }
    snprintf(cmd, sizeof(cmd), "networksetup -listallhardwareports | grep -A1 'Ethernet' | grep '%s'", device);
    run_command(cmd, buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(type, "Ethernet", len);
        return;
    }
    strncpy(type, "Unknown", len);
}

// Change MAC address (ARM)
bool change_mac_arm(const char *device, const char *mac) {
    char type[32];
    get_interface_type(device, type, sizeof(type));
    if (strcmp(type, "Wi-Fi") == 0) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sudo /usr/sbin/networksetup -setairportpower %s off", device);
        system(cmd);
        usleep(200000);
        snprintf(cmd, sizeof(cmd), "sudo /sbin/ifconfig %s ether %s 2>/dev/null", device, mac);
        int success1 = system(cmd);
        snprintf(cmd, sizeof(cmd), "sudo /usr/sbin/networksetup -setairportpower %s on", device);
        system(cmd);
        usleep(200000);
        snprintf(cmd, sizeof(cmd), "sudo /sbin/ifconfig %s ether %s 2>/dev/null", device, mac);
        int success2 = system(cmd);
        system("sudo /usr/sbin/networksetup -detectnewhardware");
        usleep(200000);
        return (success1 == 0 || success2 == 0);
    } else {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sudo /sbin/ifconfig %s ether %s 2>/dev/null", device, mac);
        int success = system(cmd);
        return (success == 0);
    }
}

// Change MAC address (Intel)
bool change_mac_intel(const char *device, const char *mac) {
    char type[32];
    get_interface_type(device, type, sizeof(type));
    if (strcmp(type, "Wi-Fi") == 0) {
        printf("WARNING: The airport command line tool is deprecated and will be removed in a future release.\n");
        printf("For diagnosing Wi-Fi related issues, use the Wireless Diagnostics app or wdutil command line tool.\n");
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sudo /System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport %s -z", device);
        system(cmd);
        sleep(2);
    }
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo /sbin/ifconfig %s ether %s 2>/dev/null", device, mac);
    int success = system(cmd);
    return (success == 0);
}

int main(int argc, char *argv[]) {
    int opt;
    int option_index = 0;
    char *mac = NULL;
    char *device = NULL;
    bool set_mac = false;
    bool show_mac_flag = false;

    static struct option long_options[] = {
        {"version", no_argument, 0, 'v'},
        {"show", required_argument, 0, 's'},
        {"config", required_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "vs:c:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v':
                print_version();
                return 0;
            case 's':
                show_mac_flag = true;
                device = optarg;
                break;
            case 'c':
                set_mac = true;
                device = optarg;
                if (optind < argc) {
                    mac = argv[optind];
                } else {
                    fprintf(stderr, "Missing new-mac-address argument.\n");
                    print_usage();
                    return 1;
                }
                break;
            case '?':
            default:
                print_usage();
                return 1;
        }
    }

    if (show_mac_flag) {
        if (!device) {
            fprintf(stderr, "Missing device argument.\n");
            print_usage();
            return 1;
        }
        show_mac(device);
        return 0;
    }

    if (set_mac) {
        if (!device || !mac) {
            fprintf(stderr, "Missing device or new-mac-address argument.\n");
            print_usage();
            return 1;
        }
        if (device_down(device)) {
            fprintf(stderr, "Device %s is down.\n", device);
            return 1;
        }
        if (!valid_mac(mac)) {
            fprintf(stderr, "Mac address is not valid.\n");
            return 1;
        }
    } else if (optind < argc) {
        device = argv[optind];
        if (!device) {
            print_usage();
            return 1;
        }
        if (device_down(device)) {
            fprintf(stderr, "Device %s is down.\n", device);
            return 1;
        }
        char macbuf[18];
        generate_mac(macbuf);
        mac = macbuf;
        set_mac = true;
    } else {
        print_usage();
        return 1;
    }

    if (set_mac) {
        char arch[32];
        get_cpu_arch(arch, sizeof(arch));
        bool change_success = false;
        if (strcmp(arch, "arm64") == 0) {
            change_success = change_mac_arm(device, mac);
        } else {
            change_success = change_mac_intel(device, mac);
        }
        char actual_mac[64] = {0};
        char cmd_show[256], buf_show[256];
        snprintf(cmd_show, sizeof(cmd_show), "/sbin/ifconfig %s | grep ether", device);
        run_command(cmd_show, buf_show, sizeof(buf_show));
        char *macptr = strstr(buf_show, "ether ");
        if (macptr) {
            macptr += 6;
            char *end = strchr(macptr, '\n');
            if (end) *end = '\0';
            strncpy(actual_mac, macptr, sizeof(actual_mac)-1);
        }
        if (strcasecmp(actual_mac, mac) == 0) {
            printf("[+] MAC address successfully set for %s: %s\n", device, actual_mac);
            return 0;
        } else {
            printf("[!] Failed to set MAC address for %s\n", device);
            printf("[=] Current MAC remains: %s\n", actual_mac);
            printf("\n[x] MAC operation failed. Possible causes:\n");
            printf("    • Insufficient privileges (try sudo)\n");
            printf("    • MAC address rejected by system\n");
            printf("    • Interface may not support spoofing\n");
            printf("    • The network stack is feeling stubborn today\n");
            printf("    • Or perhaps, fate resists your command\n");
            return 1;
        }
    }
    return 0;
} 