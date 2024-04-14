/*
 *  ipkcpd.cpp
 *  Server for Remote Calculator - IPK
 *  Copyright (C) 2023  Katerina Cepelková, xcepel03@stud.fit.vutbr.cz
 */

#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <regex>
#include <pcap.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iomanip>
#include <set>
#include <cmath>
#include <bitset>
#include <ncurses.h>
#include <syslog.h>


using namespace std;


// --- MACROS ---
#define IPv4_BITS 32 // number of bites in IPv4 adrress
#define WIN_WIDTH 72 // longest informations + bar + 2 steps between
// DHCP positions in packet
#define DHCP_YIADDR 16 // yiaddr starts at 16 byte
#define DHCP_OPTIONS 236 // options start at the 236 byte of dhcp packet
#define MAGIC_COOKIE_OFFSET 4 // magic cookie is 4 bytes long
// DHCP codes
#define DHCP_MESSAGE_TYPE 53 // DHCP option for message type
#define DHCP_OPTION_END 255 // code indicating end of dhcp file
#define ACK 5 // Message type for ACK


// --- STRUCTURES ---
struct prefix {
    string full;
    int netmask;
    uint32_t subnet;
    uint32_t mask;
    int maxHosts; 
    int occupied = 0;
    double utilization = 0;
    bool warned = false;
};


// --- FUNCTIONS ---
// Handeling opening offline packets
pcap_t *pcap_offline(string filename) {
        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t *handle = pcap_open_offline(filename.c_str(), errbuf);
        if (handle == nullptr) {
            cerr << "Error opening file: " << errbuf << endl;
        }
        return handle;
}

// Handeling opening live packets
pcap_t *pcap_online(string interface) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* handle;
    
    // Opening network interface
    handle = pcap_open_live(interface.c_str(), 65536, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Could not open device " << interface << ": " << errbuf << std::endl;
        return nullptr;
    }

    // Filtering DHCP packets
    struct bpf_program fp;
    char filter_exp[] = "udp and (port 67 or port 68)";
    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return nullptr;
    }

    // Setting filter
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return nullptr;
    }

    return handle;
}

// Transforms 4 numbers from ip adrress (1 byte) into 32 bit number
uint32_t ip_num_to_bits(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
    uint32_t bitIP = (static_cast<uint32_t>(byte1 << 24)) | (static_cast<uint32_t>(byte2 << 16)) 
                    | (static_cast<uint32_t>(byte3 << 8)) | (static_cast<uint32_t>(byte4));
    return bitIP;
}

// Takes apart ip prefix and saves its important parts (netmask, subnet, mask, maxHosts)
void prefix_analyze(string ipPrefix, vector<prefix> &ipPrefixes) {
    prefix info; 
    info.full = ipPrefix;
        
    regex ipNumbers("(\\d+).(\\d+).(\\d+).(\\d+)\\/(\\d+)");
    smatch match;
    if (regex_match(info.full, match, ipNumbers)) {
        info.netmask = stoi(match[5]); // netmask number stored in fifth submatch (after /)
    }
       
    // Making of mask (transfering numbers of IP adrress into bites -> bit shifting into mask
    info.subnet = ip_num_to_bits(static_cast<uint8_t>(stoi(match[1])), static_cast<uint8_t>(stoi(match[2])), 
                static_cast<uint8_t>(stoi(match[3])), static_cast<uint8_t>(stoi(match[4]))); 
    info.subnet = info.subnet >> (IPv4_BITS - info.netmask);
    info.subnet = info.subnet << (IPv4_BITS - info.netmask);

    info.mask = 0xFFFFFFFF << (IPv4_BITS - info.netmask);

    info.maxHosts = pow(2, (IPv4_BITS - info.netmask)) - 2; // ! -2 -> (broadcast, address)

    ipPrefixes.push_back(info);
}

// Parsing and checking arguments
int args_handle(int argc, char *argv[], string &filename, string &interface, vector<prefix> &ipPrefixes) {
    int opt; 
    while ((opt = getopt(argc, argv, "i:r:h")) != -1) {
        switch (opt) {
            case 'r':
                if (!filename.empty()) {
                    cerr << "You can not put in multiple filenames" << endl;
                    return 1;  
                }
                filename = optarg;
                break;

            case 'i':
                if (!interface.empty()) {
                    cerr << "You can not put in multiple interfaces" << endl;
                    return 1;  
                }
                interface = optarg;
                break;
	
            case 'h':
                cout << "Usage: ./dhcp-stats [-r <filename>] [-i <interface-name>] <ip-prefix> [ <ip-prefix> [ ... ] ]" << endl;
                cout << "Options:" << endl;
                cout << "  -r <filename>     - specify the input filename" << endl;
                cout << "  -i <interface>    - specify the interface" << endl;
                cout << "  -h                - display help message" << endl;
                return 0;

            default:
                cerr << "For help use./dhcp-stats -h" << endl;
                return 1;
        }
    }
    
    // Checking with regex for validity of rest (should be all IPv4 prefixes)
    regex IPv4Regex("^(((([0-1]?[0-9]?[0-9])|(2[0-4][0-9])|(25[0-5]))\\.){3}(([0-1]?[0-9]?[0-9])|(2[0-4][0-9])|(25[0-5]))\\/(([1-2]?[0-9])|3[0-2]))$");
    for (int pos = optind; pos < argc; pos++) {
        if (!regex_match(argv[pos], IPv4Regex)) {
            cerr << "Invalid IPv4 prefix" << endl;
            return 1;                  
        }
        prefix_analyze(argv[pos], ipPrefixes);
    }

    // Ip prefix is required
    if (ipPrefixes.empty()) {
        cerr << "Missing IP prefix" << endl;
        return 1; 
    }
    
    // Either filename or interface is required
    if (filename.empty() && interface.empty()) {
        cerr << "Missing arguments" << endl;
        cerr << "For help use./dhcp-stats -h" << endl;
        return 1; 
    } else if (!filename.empty() && !interface.empty()) {
        cerr << "Enter just filename or just interface" << endl;
        return 1;  
    }

    return 0;
}

// Updating all window of ncourses with prefix values
void win_update(WINDOW *win, vector<prefix> ipPrefixes) {
    int i = 2;
    string spaces(72, ' '); 
    for (const prefix &ipPrefix : ipPrefixes) {
        mvwprintw(win, i, 0, "%s", spaces.c_str()); // cleanup whole row
        mvwprintw(win, i, 2, "%s", ipPrefix.full.c_str());
        mvwprintw(win, i, 23, "%d", ipPrefix.maxHosts);
        mvwprintw(win, i, 36, "%d", ipPrefix.occupied);
        mvwprintw(win, i, 58, "%f%%", ipPrefix.utilization);
        i++;
    }
    wrefresh(win);
}

// Setting up window for stats
void win_setup(WINDOW *win, vector<prefix> ipPrefixes) {
    // Printing bar
    mvwprintw(win, 1, 2, "IP-Prefix");
    mvwprintw(win, 1, 23, "Max-hosts");
    mvwprintw(win, 1, 36, "Allocated addresses");
    mvwprintw(win, 1, 58, "Utilization");
    
    // Inicializing everything on 0
    win_update(win, ipPrefixes);
}

// Check if yiaddr is compatible with any ipPrefix, if yes updates it
void check_compatibility(uint32_t yiaddr, set<uint32_t> &ipAdresses, vector<prefix> &ipPrefixes, WINDOW *win) {
    if (ipAdresses.find(yiaddr) == ipAdresses.end()) {
        ipAdresses.insert(yiaddr);
        for (prefix &ipPrefix : ipPrefixes) {
            if ((yiaddr & ipPrefix.mask) == ipPrefix.subnet) {
                ipPrefix.occupied++;
                ipPrefix.utilization = (static_cast<double>(ipPrefix.occupied) / static_cast<double>(ipPrefix.maxHosts)) * 100;
            }
            if (ipPrefix.utilization > 50.00 && !ipPrefix.warned) {
                syslog(LOG_INFO, "Prefix %s exceeded 50%% of allocations.", ipPrefix.full.c_str());
                cout << "Prefix " << ipPrefix.full.c_str() << " exceeded 50% of allocations." << endl;
                ipPrefix.warned = true;
            }
        }
        win_update(win, ipPrefixes);
    }
}

// Locating allocated adrresses in packets
void packet_handle(const u_char *packet, set<uint32_t> &ipAdresses, vector<prefix> &ipPrefixes, WINDOW *win) {
    const struct ip *ipHeader = (struct ip*)(packet + sizeof(struct ether_header)); // IP header
    // Check for UDP header (ip protocol 17)
    if (ipHeader->ip_p == IPPROTO_UDP) {
        const struct udphdr *udpHeader = (struct udphdr*)(packet + sizeof(struct ether_header) + sizeof(struct ip)); // UDP header
        // Check for right DHCP packet (src port 67, dst port 68) -> ACK/Offer
        if (udpHeader->source == htons(67) || udpHeader->dest == htons(68)) {
            // Start of DHCP data packet
            uint8_t *dhcpPacket = (uint8_t*)(packet + sizeof(struct ether_header) + sizeof(struct ip) + sizeof(udphdr));
            const u_char *dhcpOptions = dhcpPacket + DHCP_OPTIONS + MAGIC_COOKIE_OFFSET; // start of options
            int offset = 0;
            uint8_t optionCode; // first option
            // Finding rigt message type (code 53 and type 5 = ACK)
            do {
        	optionCode = dhcpOptions[offset]; // getting option code
                if (static_cast<int>(optionCode) == DHCP_MESSAGE_TYPE && static_cast<int>(dhcpOptions[offset + 2] == ACK)) {
                        uint32_t yiaddr = ip_num_to_bits(static_cast<uint8_t>(dhcpPacket[DHCP_YIADDR]), 
                                static_cast<uint8_t>(dhcpPacket[DHCP_YIADDR + 1]),static_cast<uint8_t>(dhcpPacket[DHCP_YIADDR + 2]), 
                                static_cast<uint8_t>(dhcpPacket[DHCP_YIADDR + 3]));
                        check_compatibility(yiaddr, ipAdresses, ipPrefixes, win);
            		break;
        	    }
        	    offset += 2 + dhcpOptions[offset + 1]; // Next option = skip code + length + payload
    	    } while (optionCode != DHCP_OPTION_END);
        }
    }
}


// --- MAIN ---
int main(int argc, char *argv[]) {
    string filename, interface;
    vector<prefix> ipPrefixes;
    set<uint32_t> ipAdresses; // set of all allocated ip adresses
    struct pcap_pkthdr header;

    if (args_handle(argc, argv, filename, interface, ipPrefixes)) { 
        return 1;
    }

    openlog("dhcp-stats", LOG_PID | LOG_CONS, LOG_USER); // (cat /var/log/syslog | grep "dhcp-stats")

    if (initscr() == nullptr) {
        cerr << "Could not initialize ncourses library" << endl;
        return 1;
    }

    int height = 2 + ipPrefixes.size(); // height of window = bar + end + all ipPrefixes
    WINDOW *win = newwin(height, WIN_WIDTH, 0, 0);
    nodelay(win, TRUE); // periodic updates
    refresh();

    if (!interface.empty()) { // ONLINE
        pcap_t *handle = pcap_online(interface);
        if (handle == nullptr) {
            endwin();
            closelog();
            cerr << "Error while handeling pcap handle" << endl;
            return 1;
        }
        win_setup(win, ipPrefixes);

        // Capturing
        while (1) {
            const u_char *packet = pcap_next(handle, &header);
            packet_handle(packet, ipAdresses, ipPrefixes, win);
            wrefresh(win);
            refresh();
        }
        pcap_close(handle);
    } else if (!filename.empty()) { // OFFLINE
        pcap_t *handle = pcap_offline(filename);
        if (handle == nullptr) {
            endwin();
            closelog();
            cerr << "Error while handeling pcap handle" << endl;
            return 1;
        }
        win_setup(win, ipPrefixes);

        // Capturing
        const u_char* packet;
        while ((packet = pcap_next(handle, &header)) != nullptr) {
            packet_handle(packet, ipAdresses, ipPrefixes, win);
            wrefresh(win);
            refresh();
        }
        pcap_close(handle);
        getch();
    }

    endwin();
    closelog();
    return 0;
}
