class Network
  class << self
    attr_accessor :pass, :user, :apn
  end

  def self.getaddress(host)
    TCPSocket.getaddress(host)
  end
end

