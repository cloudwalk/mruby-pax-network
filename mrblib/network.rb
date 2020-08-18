
class Network
  NET_LINK_ETH       = 1 # Ethernet
  NET_LINK_ETH1      = 2 # USB Ethernet
  NET_LINK_WL        = 3 # GPRS,CDMA,TDSCDMA
  NET_LINK_WIFI      = 4 # Wifi
  NET_LINK_PPPOE     = 5 # ADSL
  NET_LINK_MODEM     = 6 # Modem PPP
  NET_LINK_PPPDIRECT = 7 # ppp_direct

  MEDIA_GPRS         = "gprs"
  MEDIA_WIFI         = "wifi"
  MEDIA_ETHERNET     = "ethernet"

  class << self
    attr_accessor :interface, :network_init
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
    self.network_init = @interface.init(options)
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

  def self.socket_open?
    ret = (!! DaFunk::PaymentChannel.client&.connected?)
    @con_check = Time.now + 10 if ret
    ret
  end

  def self.connected?
    if self.started?
      return -3307 unless initialized?
      if @con && @con >= 0 && (@con_check.is_a?(Time) && (@con_check > Time.now) || socket_open?)
        @con
      else
        @con_check = Time.now + 10
        @con = @interface.connected?
      end
      @con
    else
      -3307
    end
  end

  def self.connect(*options)
    return -3307 unless initialized?
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
    klass = (if_klass || @interface)
    if klass == Network::Gprs
      "eth0"
    elsif klass == Network::Ethernet
      "eth0"
    elsif klass == Network::Wifi
      "wlan0"
    else
      "eth0"
    end
  end

  def self.initialized?
    self.network_init == 0
  end

  def self.network_init
    value = Device::Setting.network_init
    if ! value.to_s.empty? && value.to_i != @network_init
      @network_init = value.to_i
    end
    @network_init
  end

  def self.network_init=(value)
    Device::Setting.network_init = value
    @network_init = value
  end

  def self.method_missing(method, *args, &block)
    if @interface.respond_to? method
      @interface.send(method, *args, &block)
    else
      super
    end
  end
end
