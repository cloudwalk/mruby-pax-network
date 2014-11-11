
class Network
  Wifi = ::Wifi

  class Wifi
    class << self
      attr_accessor :bssid, :essid, :rssi, :authentication, :password, :essid, :bssid, :channel, :mode, :cipher
      attr_reader :media
      @media = :wifi
    end

    AUTH_NONE_OPEN       = "1"
    AUTH_NONE_WEP        = "2"
    AUTH_NONE_WEP_SHARED = "3"
    AUTH_IEEE8021X       = "4"
    AUTH_WPA_PSK         = "5"
    AUTH_WPA_EAP         = "6"
    AUTH_WPA_WPA2_PSK    = "7"
    AUTH_WPA_WPA2_EAP    = "8"
    AUTH_WPA2_PSK        = "9"
    AUTH_WPA2_EAP        = "10"

    PARE_CIPHERS_NONE   = "0"
    PARE_CIPHERS_WEP64  = "1"
    PARE_CIPHERS_WEP128 = "2"
    PARE_CIPHERS_WEPX   = "04"
    PARE_CIPHERS_CCMP   = "10"
    PARE_CIPHERS_TKIP   = "20"

    MODE_IBSS    = "1"
    MODE_STATION = "0"

    AUTHENTICATIONS = {
      :open         => AUTH_NONE_OPEN,
      :wep_none     => AUTH_NONE_WEP,
      :wep_shared   => AUTH_NONE_WEP_SHARED,
      :iEEE8021X    => AUTH_IEEE8021X,
      :wpa_psk      => AUTH_WPA_PSK,
      :wpa_eap      => AUTH_WPA_EAP,
      :wpa_wpa2_psk => AUTH_WPA_WPA2_PSK,
      :wpa_wpa2_eap => AUTH_WPA_WPA2_EAP,
      :wpa2_psk     => AUTH_WPA2_PSK,
      :wpa2_eap     => AUTH_WPA2_EAP
    }

    def self.init(options = {})
      @essid          = options[:essid].to_s
      @bssid          = options[:bssid].to_s
      @mode           = options[:mode].to_s
      @authentication = options[:authentication].to_s
      @cipher         = options[:cipher].to_s
      @password       = options[:password].to_s
      @channel        = options[:channel].to_s
      self.start
    end

  end
end

