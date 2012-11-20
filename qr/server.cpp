#include "main.h"
#include "server.h"
#include "Client.h"
countryRegion countries[] = {
							{"BI","Burundi",REGIONID_AFRICA|REGIONID_CENTRAL_AFRICA},
							{"CM","Cameroon",REGIONID_AFRICA|REGIONID_CENTRAL_AFRICA},
							{"CF","Central African Republic",REGIONID_AFRICA|REGIONID_CENTRAL_AFRICA},
							{"TD","Chad",REGIONID_AFRICA|REGIONID_CENTRAL_AFRICA},
							{"CG","Congo",REGIONID_AFRICA|REGIONID_CENTRAL_AFRICA},
							{"GQ","Equatorial Guinea",REGIONID_AFRICA|REGIONID_CENTRAL_AFRICA},
							{"RW","Rwanda",REGIONID_AFRICA|REGIONID_CENTRAL_AFRICA},
							{"DJ","Djibouti",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"ER","Eritrea",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"ET","Ethiopia",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"KE","Kenya",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"SC","Seychelles",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"SO","Somalia",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"SH","St. Helena",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"SD","Sudan",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"TZ","Tanzania",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"UG","Uganda",REGIONID_AFRICA|REGIONID_EAST_AFRICA},
							{"DZ","Algeria",REGIONID_AFRICA|REGIONID_NORTH_AFRICA},
							{"EG","Egypt",REGIONID_AFRICA|REGIONID_NORTH_AFRICA},
							{"LY","Libya",REGIONID_AFRICA|REGIONID_NORTH_AFRICA},
							{"MA","Morocco",REGIONID_AFRICA|REGIONID_NORTH_AFRICA},
							{"TN","Tunisia",REGIONID_AFRICA|REGIONID_NORTH_AFRICA},
							{"AO","Angola",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"BW","Botswana",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"BV","Bouvet Island",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"KM","Comoros",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"HM","Heard and McDonald Islands",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"LS","Lesotho",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"MG","Madagascar",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"MW","Malawi",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"MU","Mauritius",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"YT","Mayotte",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"MZ","Mozambique",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"NA","Namibia",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"RE","Reunion",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"ZA","South Africa",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"SZ","Swaziland",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"ZM","Zambia",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"ZW","Zimbabwe",REGIONID_AFRICA|REGIONID_SOUTH_AFRICA},
							{"BJ","Benin",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"BF","Burkina Faso",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"CV","Cape Verde",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"CI","Cote D`ivoire",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"GA","Gabon",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"GM","Gambia",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"GH","Ghana",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"GN","Guinea",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"GW","Guinea-Bissau",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"LR","Liberia",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"ML","Mali",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"MR","Mauritania",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"NE","Niger",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"NG","Nigeria",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"ST","Sao Tome and Principe",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"SN","Senegal",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"SL","Sierra Leone",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"TG","Togo",REGIONID_AFRICA|REGIONID_WEST_AFRICA},
							{"AI","Anguilla",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"AG","Antigua and Barbuda",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"AW","Aruba",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"BS","Bahamas",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"BB","Barbados",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"BM","Bermuda",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"KY","Cayman Islands",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"CU","Cuba",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"DM","Dominica",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"DO","Dominican Republic",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"GD","Grenada",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"GP","Guadeloupe",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"HT","Haiti",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"JM","Jamaica",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"MQ","Martinique",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"MS","Montserrat",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"AN","Netherlands Antilles",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"PR","Puerto Rico",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"VC","Saint Vincent and The Grenadines",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"KN","St Kitts-Nevis",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"LC","St Lucia",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"TT","Trinidad & Tobago",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"TC","Turks & Caicos Islands",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"VG","Virgin Islands (British)",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"VI","Virgin Islands (US)",REGIONID_AMERICAS|REGIONID_CARIBBEAN},
							{"BZ","Belize",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA},
							{"CR","Costa Rica",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA},
							{"SV","El Salvador",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA},
							{"GT","Guatemala",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA},
							{"HN","Honduras",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA},
							{"MX","Mexico",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA}, //gamespy says this not me!!
							{"NI","Nicaragua",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA},
							{"PA","Panama",REGIONID_AMERICAS|REGIONID_CENTRAL_AMERICA},
							{"CA","Canada",REGIONID_AMERICAS|REGIONID_NORTH_AMERICA},
							{"GL","Greenland",REGIONID_AMERICAS|REGIONID_NORTH_AMERICA},
							{"PM","St. Pierre and Miquelon",REGIONID_AMERICAS|REGIONID_NORTH_AMERICA},
							{"US","United States",REGIONID_AMERICAS|REGIONID_NORTH_AMERICA},
							{"UM","US Minor Outlying Islands",REGIONID_AMERICAS|REGIONID_NORTH_AMERICA},
							{"AR","Argentina",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"BO","Bolivia",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"BR","Brazil",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"CL","Chile",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"CO","Colombia",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"EC","Ecuador",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"GF","S. Georgia and S. Sandwich Islands",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"SR","Suriname",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"UY","Uruguay",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"VE","Venezuela",REGIONID_AMERICAS|REGIONID_SOUTH_AMERICA},
							{"CN","China",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"HK","Hong Kong",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"JP","Japan",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"MO","Macao",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"MN","Mongolia",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"KP","North Korea",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"KR","South Korea",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"TW","Taiwan",REGIONID_ASIA|REGIONID_EAST_ASIA},
							{"AS","American Samoa",REGIONID_ASIA|REGIONID_PACIFIC},
							{"AU","Australia",REGIONID_ASIA|REGIONID_PACIFIC},
							{"CK","Cook Islands",REGIONID_ASIA|REGIONID_PACIFIC},
							{"FJ","Fiji",REGIONID_ASIA|REGIONID_PACIFIC},
							{"PF","French Polynesia",REGIONID_ASIA|REGIONID_PACIFIC},
							{"GU","Guam",REGIONID_ASIA|REGIONID_PACIFIC},
							{"KI","Kiribati",REGIONID_ASIA|REGIONID_PACIFIC},
							{"MH","Marshall Islands",REGIONID_ASIA|REGIONID_PACIFIC},
							{"FM","Micronesia",REGIONID_ASIA|REGIONID_PACIFIC},
							{"NR","Nauru",REGIONID_ASIA|REGIONID_PACIFIC},
							{"NC","New Caledonia",REGIONID_ASIA|REGIONID_PACIFIC},
							{"NZ","New Zealand",REGIONID_ASIA|REGIONID_PACIFIC},
							{"NU","Niue",REGIONID_ASIA|REGIONID_PACIFIC},
							{"NF","Norfolk Island",REGIONID_ASIA|REGIONID_PACIFIC},
							{"MP","Northern Mariana Islands",REGIONID_ASIA|REGIONID_PACIFIC},
							{"PG","Papua New Guinea",REGIONID_ASIA|REGIONID_PACIFIC},
							{"PN","Pitcairn Islands",REGIONID_ASIA|REGIONID_PACIFIC},
							{"EH","Samoa",REGIONID_ASIA|REGIONID_PACIFIC},
							{"SB","Solomon Islands",REGIONID_ASIA|REGIONID_PACIFIC},
							{"TO","Tonga",REGIONID_ASIA|REGIONID_PACIFIC}, //duplicate on gamespy :X
							{"TK","Tonga",REGIONID_ASIA|REGIONID_PACIFIC},
							{"TV","Tuvalu",REGIONID_ASIA|REGIONID_PACIFIC},
							{"VU","Vanuatu",REGIONID_ASIA|REGIONID_PACIFIC},
							{"WF","Wallis and Futuna Islands",REGIONID_ASIA|REGIONID_PACIFIC},
							{"AF","Afghanistan",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"BD","Bangladesh",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"BT","Bhutan",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"IO","British Indian Ocean Territory",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"IN","India",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"MV","Maldives",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"NP","Nepal",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"PK","Pakistan",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"LK","Sri Lanka",REGIONID_ASIA|REGIONID_SOUTH_ASIA},
							{"BN","Brunei Darussalam",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"KH","Cambodia",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"CX","Christmas Islands",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"CC","Cocos (Keeling Islands)",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"TP","East Timor",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"ID","Indonesia",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"LA","Laos",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"MY","Malaysia",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"MM","Myanmar",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"PW","Palau",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"PH","Philippines",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"SG","Singapore",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"TH","Thailand",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"VN","Vietnam",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"PH","Philippines",REGIONID_ASIA|REGIONID_SOUTH_EAST_ASIA},
							{"EE","Estonia",REGIONID_EUROPE|REGIONID_BALTIC_STATES},
							{"LV","Latvia",REGIONID_EUROPE|REGIONID_BALTIC_STATES},
							{"LT","Lithuania",REGIONID_EUROPE|REGIONID_BALTIC_STATES},
							{"AM","Armenia",REGIONID_EUROPE|REGIONID_CIS},
							{"AZ","Azerbaijan",REGIONID_EUROPE|REGIONID_CIS},
							{"BY","Belarus",REGIONID_EUROPE|REGIONID_CIS},
							{"GE","Georgia",REGIONID_EUROPE|REGIONID_CIS},
							{"KZ","Kazakstan",REGIONID_EUROPE|REGIONID_CIS},
							{"KG","Kyrgyzstan",REGIONID_EUROPE|REGIONID_CIS},
							{"MD","Moldova",REGIONID_EUROPE|REGIONID_CIS},
							{"RU","Russian Federation",REGIONID_EUROPE|REGIONID_CIS},
							{"TJ","Tajikistan",REGIONID_EUROPE|REGIONID_CIS},
							{"TM","Turkmenistan",REGIONID_EUROPE|REGIONID_CIS},
							{"UA","Ukraine",REGIONID_EUROPE|REGIONID_CIS},
							{"UZ","Uzbekistan",REGIONID_EUROPE|REGIONID_CIS},
							{"CZ","Czech Republic",REGIONID_EUROPE|REGIONID_EASTERN_EUROPE},
							{"HU","Hungary",REGIONID_EUROPE|REGIONID_EASTERN_EUROPE},
							{"RO","Romania",REGIONID_EUROPE|REGIONID_EASTERN_EUROPE},
							{"SK","Slovak Republic",REGIONID_EUROPE|REGIONID_EASTERN_EUROPE},
							//middle east under europe?
							{"BH","Bahrain",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"IR","Iran",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"IQ","Iraq",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"IL","Israel/Occupied Territories",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"JO","Jordan",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"KW","Kuwait",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"LB","Lebanon",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"OM","Oman",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"QA","Qatar",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"SA","Saudi Arabia",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"SY","Syria",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"AE","United Arab Emirates",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							{"YE","Yemen",REGIONID_EUROPE|REGIONID_MIDDLE_EAST},
							//
							{"AL","Albania",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"BA","Bosnia-Herzegovina",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"BG","Bulgaria",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"HR","Croatia",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"CY","Cyprus",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"GR","Greece",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"MK","Macedonia",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"MT","Malta",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"SI","Slovenia",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"TR","Turkey",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"YU","Yugoslavia",REGIONID_EUROPE|REGIONID_SOUTH_EAST_EUROPE},
							{"AD","Andorra",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"AT","Austria",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"BE","Belgium",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"DK","Denmark",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"FO","Faroe Islands",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"FI","Finland",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"FR","France",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"DE","Germany",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"GI","Gibraltar",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"IS","Iceland",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"IR","Ireland",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"IT","Italy",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"LI","Liechtenstein",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"LU","Luxembourg",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"MC","Monaco",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"NL","Netherlands",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"NO","Norway",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"PT","Portugal",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"SM","San Marino",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"ES","Spain",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"SJ","Svalbard and Jan Mayen Islands",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"SE","Sweden",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"CH","Switzerland",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"UK","United Kingdom",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"VA","Vatican",REGIONID_EUROPE|REGIONID_WESTERN_EUROPE},
							{"ZZ","Unknown",0},
							};
extern serverInfo server;
void deleteClient(Client *client) {
	std::list<Client *>::iterator iterator;
	iterator=server.client_list.begin();
	while(iterator != server.client_list.end()) {
		if(*iterator==client) {
			iterator = server.client_list.erase(iterator);
			delete client;
		} else
		iterator++;

	}
}
Client *find_user(struct sockaddr_in *peer) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	struct sockaddr_in *userpeer;
	while(iterator != server.client_list.end()) {
		user=*iterator;
		userpeer = user->getSockAddr();
		if((userpeer->sin_addr.s_addr == peer->sin_addr.s_addr) && (userpeer->sin_port == peer->sin_port)) {
			return user;
		}
		iterator++;
	}
	return NULL;
}

Client *find_user(uint32_t ip, uint16_t port) {
	std::list<Client *>::iterator iterator=server.client_list.begin();
	Client *user;
	while(iterator != server.client_list.end()) {
		user=*iterator;
		if(user->getServerAddress() == ip && user->getServerPort() == port) {
			return user;
		}
		iterator++;
	}
	return NULL;
}
char *geoIPConvert(char *name) {
	struct geoipConvData {
		const char *geoIPName;
		const char *gsName;
	} geocountries[] = {{"GB","UK"}};
	for(int i=0;i<sizeof(geocountries)/sizeof(geoipConvData);i++) {
		if(strcmp(name,geocountries[i].geoIPName) == 0) {
			return (char *)geocountries[i].gsName;
		}
	}
	return name;
}
countryRegion *findCountryByName(char *name) {
	int size = sizeof(countries)/sizeof(countryRegion);
	if(name == NULL) goto end;
	name = geoIPConvert(name);
	for(int i=0;i<size;i++) {
		if(strcmp(countries[i].countrycode,name) == 0) {
			return &countries[i];
		}
	}
end:
	return &countries[size-1];
}
