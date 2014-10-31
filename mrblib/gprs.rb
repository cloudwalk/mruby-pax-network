class Network
  class Gprs
    class << self
      attr_accessor :password, :user, :apn
      attr_reader :media
      @media = :gprs
    end
  end
end
