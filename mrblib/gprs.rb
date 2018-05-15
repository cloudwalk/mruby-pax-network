
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

    def self.get_sim_id(serial = nil)
      if serial
        response = serial.command("AT+CCID\r", 33)
        if result = response.to_s.match(/\+CCID: (.+)/)
          result[1]
        end
      else
        cmd_at("/dev/mux1", 115200, 8,"N", 1) do |serial|
          response = serial.command("AT+CCID\r", 33)
          if result = response.to_s.match(/\+CCID: (.+)/)
            result[1]
          end
        end
      end
    end

    def self.select_network(imsi_id)
      cmd_at("/dev/mux1", 115200, 8, "N", 1, 20000) do |serial|
        serial.command("AT+CMEE=2\r", 200) # Enable errors
        response = serial.command("AT+COPS=0,2\r", 200)
        ContextLog.info "AT+COPS=0,2: #{response}"
        return response unless response.to_s.match(/\OK/)
        response = serial.command("AT+COPS=4,2,\"#{imsi_id}\"\r", 200)
        ContextLog.info "AT+COPS=4,2: #{response}"
        unless response.to_s.match(/\OK/)
          response = serial.command("AT+COPS=1,2,\"#{imsi_id}\"\r", 200)
          ContextLog.info "AT+COPS=1,2: #{response}"
          return response unless response.to_s.match(/\OK/)
        end
        response = serial.command("AT+CGATT=1\r", 200)
        ContextLog.info "AT+CGATT=1: #{response}"
        response
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
