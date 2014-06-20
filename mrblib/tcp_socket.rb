class TCPSocket
  CMD_TO_SET       = 2
  CMD_EVENT_GET    = 7
  SOCK_EVENT_ERROR = 1<<4

  class << self
    attr_accessor :default_timeout
  end

  self.default_timeout = 120000

  attr_accessor :fd, :timeout

  def self.open(*args)
    self.new(*args)
  end

  def timeout=(miliseconds = self.class.default_timeout)
    ioctl(CMD_TO_SET, miliseconds)
    @timeout = miliseconds
  end

  def write(mesg)
    send(mesg, 0)
  end

  # TODO Scalone refactor need
  # TODO Scalone could be a problem to time dependency
  # TODO Scalone Maybe implement in C with TimerCheck
  def read(length, outbuf=nil)
    time_start = Time.now
    buf = ""
    while (buf.size > length)
      buf << recv(1)
      return buf if (Time.now - time_start) > (self.timeout / 1000)
    end
  end

  def getaddress(host)
    self.class.getaddress(host)
  end

  def closed?
    if (@fd >= 0)
      return false if (ioctl(CMD_EVENT_GET, SOCK_EVENT_ERROR) == 0)
    end
    true
  end
end

