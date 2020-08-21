using Microsoft.EntityFrameworkCore;
using RemoteWasher.DAO;

namespace RemoteWasher
{
    public class ApplicationContext : DbContext
    {
        public ApplicationContext(DbContextOptions<ApplicationContext> options) : base(options)
        {

        }

        public DbSet<User> Users { get; set; }

        public DbSet<DeviceStatus> DeviceStatuses { get; set; }

        protected override void OnConfiguring(DbContextOptionsBuilder optionsBuilder)
        {
            //optionsBuilder.UseNpgsql("Host=192.168.20.2;Port=5432;Database=washer;Username=washer;Password=mk1637gsx");
        }
    }
}
