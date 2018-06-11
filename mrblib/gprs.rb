
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

    def self.select_attach_network(imsi_id)
      commands = ["AT+COPS=0,2","AT+COPS=4,2,\"#{imsi_id}\""]

      cmd_at("/dev/mux1", 115200, 8, "N", 1, 40000) do |serial|
        response = {}
        commands.each do |command|
          response = self.send_at_command(serial, command)
          return response unless response[:result] == "OK"
        end
        response
      end
    end

    def self.send_at_command(serial, command)
      response = {
        :result => String.new,
        :reason => String.new
      }
      3.times do
        result = serial.command("#{command}\r", 200)
        # ContextLog.info "send_at_command: #{command},result: #{result}"
        if result.to_s.match(/\OK/)
          response[:result] = "OK"
          response[:reason] = ""
          break
        else
          response[:result] = "ERROR"
          response[:reason] = result
        end
      end
      response
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
