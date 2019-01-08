
class Network
  NET_LINK_ETH       = 1 # Ethernet
  NET_LINK_ETH1      = 2 # USB Ethernet
  NET_LINK_WL        = 3 # GPRS,CDMA,TDSCDMA
  NET_LINK_WIFI      = 4 # WiFi
  NET_LINK_PPPOE     = 5 # ADSL
  NET_LINK_MODEM     = 6 # Modem PPP
  NET_LINK_PPPDIRECT = 7 # ppp_direct

  MEDIA_GPRS         = "gprs"
  MEDIA_WIFI         = "wifi"
  MEDIA_ETHERNET     = "ethernet"

  class << self
    attr_accessor :interface
  end

  def self.started?
    ! @interface.nil?
  end

  def self.ping(host, timeout)
    _ping TCPSocket.getaddress(host), timeout
  end

  def self.init(media, options)
    self.configure(media, options)
    @con = 1
    # If GPRS add some seconds to avoid SDK connected call
    # because that call could block the entire POS execution
    if @interface == Network::Gprs
      @con_check = (Time.now + 20)
    else
      @con_check = (Time.now + 10)
    end
    @interface.init(options)
  end

  def self.configure(media, options)
    case media.to_s.downcase.strip
    when MEDIA_GPRS
      @interface = Network::Gprs
    when MEDIA_WIFI
      @interface = Network::Wifi
    when MEDIA_ETHERNET
      @interface = Network::Ethernet
    else
      raise ArgumentError, "Media \"#{media}\" not supported"
    end
  end

  # TODO Scalone: Raise error if problem or do not exists
  def self.dhcp_client_start
    self._dhcp_client_start(@interface.type)
  end

  def self.dhcp_client_check
    self._dhcp_client_check(@interface.type)
  end

  def self.connected?
    if self.started?
      if @con && @con >= 0 && @con_check.is_a?(Time) && (@con_check > Time.now)
        @con
      else
        @con_check = Time.now + 5
        @con = @interface.connected?
      end
      @con
    else
      -3307
    end
  end

  def self.connect(*options)
    @con = 1
    @interface.connect(*options)
  end

  def self.disconnect
    @con = nil
    @interface && @interface.disconnect
  end

  def self.power(*options)
    @con = nil if [options].flatten.first == 0
    @interface.power(*options)
  end

  def self.mac_address(if_klass = nil)
    if (char = self._mac_address(ifname(if_klass)))
      char.bytes.map { |byte| byte.to_s(16).upcase }.join(":")
    end
  end

  def self.ifname(if_klass = nil)
    case (if_klass || @interface)
    when Network::Gprs
      "wlan1"
    when Network::Ethernet
      "eth0"
    when Network::Wifi
      "wlan0"
    else
      "eth0"
    end
  end

  def self.method_missing(method, *args, &block)
    if @interface.respond_to? method
      @interface.send(method, *args, &block)
    else
      super
    end
  end
end
