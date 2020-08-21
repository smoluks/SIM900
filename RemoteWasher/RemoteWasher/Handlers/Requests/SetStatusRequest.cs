using MediatR;

namespace RemoteWasher.Handlers.Requests
{
    public class SetStatusRequest : IRequest
    {
        public int DeviceId { get; internal set; }
        public byte SygnalLevel { get; internal set; }
    }
}
