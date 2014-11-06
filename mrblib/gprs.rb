class Network
  class Gprs
    class << self
      attr_accessor :password, :user, :apn
      attr_reader :media
      @media = :gprs
    end

    def self.init(options = {})
      @apn      = options[:apn]
      @user     = options[:user]
      @password = options[:password]
      @apn      = options[:apn]
      @apn      = options[:apn]
      self.start
    end
  end
end
