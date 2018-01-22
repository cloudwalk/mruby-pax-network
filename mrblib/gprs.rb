
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
      self.start
    end

    def self.type
      Network::NET_LINK_WL
    end

    def self.cell
      cmd_at("/dev/mux1", 115200, 8,"N", 1) do |serial|
        @sim_id ||= self.get_sim_id(serial)
        {
          :mcc => self.mcc(serial),
          :mnc => self.mnc(serial),
          :lac => self.lac(serial),
          :cell_id => self.cell_id(serial),
        }
      end
    end

    def self.mcc(serial)
      if result = serial.command("AT+cimi\r", 40).to_s.match(/\+CIMI: (.+)/)
        @cimi = result[1]
        result[1][0..2]
      else
        @cimi[0..2] if @cimi
      end
    end

    def self.mnc(serial)
      if result = serial.command("AT+cimi\r", 40).to_s.match(/\+CIMI: (.+)/)
        @cimi = result[1]
        result[1][3..4]
      else
        @cimi[3..4] if @cimi
      end
    end

    def self.lac(serial)
      serial.command("AT+CREG=2\r", 16)
      if result = serial.command("AT+CREG?\r", 43).to_s.match(/"(.+)","(.+)"/)
        @creg = result[0]
        result[1]
      else
        if result = @creg.to_s.match(/"(.+)","(.+)"/)
          result[1]
        end
      end
    end

    def self.cell_id(serial)
      serial.command("AT+CREG=2\r", 16)
      if result = serial.command("AT+CREG?\r", 43).to_s.match(/"(.+)","(.+)"/)
        @creg = result[0]
        result[2]
      else
        if result = @creg.to_s.match(/"(.+)","(.+)"/)
          result[2]
        end
      end
    end

    def self.sim_id
      @sim_id ||= self.get_sim_id
    end

    private
    def self.get_sim_id(serial = nil)
      block = Proc.new do |serial|
        response = serial.command("AT+CCID\r", 33)
        if result = response.to_s.match(/\+CCID: (.+)/)
          result[1]
        end
      end

      if serial
        block.call(serial)
      else
        cmd_at("/dev/mux1", 115200, 8,"N", 1) do |serial|
          block.call(serial)
        end
      end
    end

    def self.cmd_at(*args, &block)
      if PAX::Network.interface == PAX::Network::Gprs
        serial = PAX::Serial.new(*args)
        block.call(serial)
      end
    ensure
      serial.close if serial && serial.respond_to?(:close)
    end
  end
end

