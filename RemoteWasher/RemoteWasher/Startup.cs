using MediatR;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.EntityFrameworkCore;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using RemoteWasher.Services;
using RemoteWasher.Services.Interfaces;
using System.Threading;

namespace RemoteWasher
{
    public class Startup
    {
        public IConfiguration Configuration { get; }

        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        // This method gets called by the runtime. Use this method to add services to the container.
        public void ConfigureServices(IServiceCollection services)
        {
            services.AddControllers();

            services.AddMediatR(typeof(Startup));

            services.AddSingleton<ITelegramService, TelegramService>();

            services.AddDbContext<ApplicationContext>(options =>
                options.UseNpgsql("Host=192.168.20.2;Port=5432;Database=washer;Username=washer;Password=mk1637gsx"));
        }

        // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
        public void Configure(IApplicationBuilder app, IWebHostEnvironment env, IHostApplicationLifetime appLifetime)
        {
            appLifetime.ApplicationStarted.Register(() => LogAppStarted(app));
            appLifetime.ApplicationStopping.Register(() => LogAppStopping(app));
            appLifetime.ApplicationStopped.Register(() => LogAppStopped(app));

            if (env.IsDevelopment())
            {
                app.UseDeveloperExceptionPage();
            }

            //app.UseHttpsRedirection();

            app.UseRouting();

            //app.UseAuthorization();

            app.UseEndpoints(endpoints =>
            {
                endpoints.MapControllers();
            });            
        }

        private void LogAppStarted(IApplicationBuilder app)
        {
            var telegramService = app.ApplicationServices.GetService<ITelegramService>();
            telegramService.SendMessageAsync("App started", CancellationToken.None);
        }

        private void LogAppStopping(IApplicationBuilder app)
        {
            var telegramService = app.ApplicationServices.GetService<ITelegramService>();
            telegramService.SendMessageAsync("App stopping", CancellationToken.None);
        }

        private void LogAppStopped(IApplicationBuilder app)
        {
            var telegramService = app.ApplicationServices.GetService<ITelegramService>();
            telegramService.SendMessageAsync("App stopped", CancellationToken.None);
        }
    }
}
