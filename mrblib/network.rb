class Network
  class << self
    attr_accessor :interface
  end

  def self.ping(host, timeout)
    _ping TCPSocket.getaddress(host), timeout
  end

  def self.init(media, options)
    if media == :gprs
      @interface = Network::Gprs
    else
      @interface = Network::Wifi
    end
    @interface.init(options)
  end

  def self.method_missing(method, *args, &block)
    if @interface.respond_to? :method
      @interface.send(method, *args, &block)
    else
      super
    end
  end
end

