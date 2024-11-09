/*
 * Quang: day la retardnet module, module nay dung netfilter 
 * de phuc vu network emulation cho 4 loai QoS
 * 1. throughput
 * 2. loss
 * 3. delay
 * 4. jitter
 * mo.i nguo`i do.c code nhe', co' va`i do`ng ik ma`.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/ip.h>

#define NETLINK_USER 31

#define MAX_RULES 4

#ifdef FIX_PLR_5
#include <linux/random.h>
#endif

typedef unsigned int (*rule_func_t)(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

static unsigned int g_throughput_leg(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return NF_ACCEPT;
}

static unsigned int g_loss_leg(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
#ifdef FIX_PLR_5
    unsigned int random_value = get_random_u32() % 100;
    return random_value < 5 ? NF_DROP : NF_ACCEPT;
#endif
    return NF_ACCEPT;
}

static unsigned int g_delay_leg(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return NF_ACCEPT;
}

static unsigned int g_jitter_leg(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    return NF_ACCEPT;
}

rule_func_t g_exe_legs[MAX_RULES] = {
    g_loss_leg,
    g_delay_leg,
    g_jitter_leg,
    g_throughput_leg};

struct sock *nl_sk = NULL;
static struct nf_hook_ops nfho;
static __be32 target_ip = 0; // target IP for applying rules

// Netfilter hook function
static unsigned int hook_func(void *priv, struct sk_buff *skb,
                              const struct nf_hook_state *state)
{
    struct iphdr *ip_header = (struct iphdr *)skb_network_header(skb);

    int accept_weight = 0;
    int drop_weight = 0;
    int repeat_weight = 0;
    int stolen_weight = 0;

    // Check if we should drop packets from the target IP
    if (target_ip && ip_header->saddr == target_ip)
    {
        for (int i = 0; i < MAX_RULES; i++)
        {
            unsigned int result = g_exe_legs[i](priv, skb, state);
            switch (result)
            {
            case NF_ACCEPT:
                accept_weight += 1;
                break;
            case NF_DROP:
                drop_weight += 3;
                break;
            case NF_REPEAT:
                repeat_weight += 2;
                break;
            case NF_STOLEN:
                stolen_weight += 2;
                break;
            }
        }

        // Determine final action based on the highest weight
        if (drop_weight >= accept_weight && drop_weight >= repeat_weight && drop_weight >= stolen_weight)
        {
            return NF_DROP;
        }
        else if (accept_weight >= repeat_weight && accept_weight >= stolen_weight)
        {
            return NF_ACCEPT;
        }
        else if (repeat_weight >= stolen_weight)
        {
            return NF_REPEAT;
        }
        else
        {
            return NF_STOLEN;
        }
    }
    return NF_ACCEPT;
}

// Netlink message handler
static void netlink_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    __be32 new_target_ip;
    nlh = (struct nlmsghdr *)skb->data;
    memcpy(&new_target_ip, nlmsg_data(nlh), sizeof(__be32));

    target_ip = new_target_ip; // Update target IP
    printk(KERN_INFO "Updated target IP to apply rules: %pI4\n", &target_ip);
}

// Module init
static int __init rtnet_init(void)
{
    // Initialize Netfilter hook
    nfho.hook = hook_func;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho);

    // Initialize Netlink socket
    struct netlink_kernel_cfg cfg = {
        .input = netlink_recv_msg,
    };
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk)
    {
        printk(KERN_ALERT "Error creating Netlink socket\n");
        return -10;
    }

    printk(KERN_INFO "Retardnet module loaded.\n");
    return 0;
}

// Module exit
static void __exit rtnet_exit(void)
{
    nf_unregister_net_hook(&init_net, &nfho);
    netlink_kernel_release(nl_sk);
    printk(KERN_INFO "Retardnet module unloaded.\n");
}

module_init(rtnet_init);
module_exit(rtnet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Retard team");
MODULE_DESCRIPTION("Retardnet Kernel Module with Dynamic Rules via Netlink");
