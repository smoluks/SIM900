using System.Threading;
using System.Threading.Tasks;

namespace RemoteWasher.Services.Interfaces
{
    public interface ITelegramService
    {
        Task SendMessageAsync(string text, CancellationToken cancellationToken);
    }
}
