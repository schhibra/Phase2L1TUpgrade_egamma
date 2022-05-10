library std;
use std.textio.all;
use std.env.all;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;

use work.regionizer_data.all;


entity testbench is
--  Port ( );
end testbench;

architecture Behavioral of testbench is
    constant NREGIONS : natural := NPFREGIONS;

    signal clk : std_logic := '0';
    signal rst : std_logic := '0';
    signal start, ready, idle, done : std_logic;
    signal newevent, newevent_out : std_logic;

    signal p_in:  w72s(NMUFIBERS-1 downto 0) := (others => (others => '0'));
    signal p_out: w72s(NREGIONS-1 downto 0) := (others => (others => '0'));
    signal v_out: std_logic_vector(NREGIONS-1 downto 0) := (others => '0');

    file Fi : text open read_mode is "input-mu.txt";
    file Fo : text open write_mode is "output-mu-vhdl_tb.txt";


begin
    clk  <= not clk after 1.25 ns;
    
    uut : entity work.mu_regionizer
        generic map(ETA_CENTER => ENDCAP_ETA_CENTER)
        port map(ap_clk => clk, 
                 ap_rst => rst, 
                 ap_start => start,
                 ap_ready => ready,
                 ap_idle =>  idle,
                 ap_done => done,
                 mu_in_0_V => p_in( 0),
                 mu_in_1_V => p_in( 1),
                 mu_out_0_V => p_out(0),
                 mu_out_1_V => p_out(1),
                 mu_out_2_V => p_out(2),
                 mu_out_3_V => p_out(3),
                 mu_out_4_V => p_out(4),
                 mu_out_5_V => p_out(5),
                 mu_out_6_V => p_out(6),
                 mu_out_7_V => p_out(7),
                 mu_out_8_V => p_out(8),
                 mu_out_valid_0 => v_out(0),
                 mu_out_valid_1 => v_out(1),
                 mu_out_valid_2 => v_out(2),
                 mu_out_valid_3 => v_out(3),
                 mu_out_valid_4 => v_out(4),
                 mu_out_valid_5 => v_out(5),
                 mu_out_valid_6 => v_out(6),
                 mu_out_valid_7 => v_out(7),
                 mu_out_valid_8 => v_out(8),
                 newevent => newevent,
                 newevent_out => newevent_out
             );
   

    runit : process 
        variable remainingEvents : integer := 5;
        variable frame : integer := 0;
        variable Li, Lo : line;
        variable itest, iobj : integer;
        variable gpart : glbparticle;
    begin
        rst <= '1';
        wait for 5 ns;
        rst <= '0';
        start <= '0';
        p_in <= (others => (others => '0'));
        wait until rising_edge(clk);
        while remainingEvents > 0 loop
            if not endfile(Fi) then
                readline(Fi, Li);
                read(Li, itest);
                read(Li, iobj); if (iobj > 0) then newevent <= '1'; else newevent <= '0'; end if;
                for i in 0 to NMUFIBERS-1  loop
                    read(Li, iobj); gpart.pt   := (to_unsigned(iobj, 14));
                    read(Li, iobj); gpart.eta  := (to_signed(iobj, 12));
                    read(Li, iobj); gpart.phi  := (to_signed(iobj, 11));
                                    gpart.rest := (others => '0');
                    p_in(i) <= glbparticle_to_w72(gpart);
                end loop;
                start <= '1';
             else
                remainingEvents := remainingEvents - 1;
                newevent <= '0';
                p_in <= (others => (others => '0'));
                start <= '1';
            end if;
           -- ready to dispatch ---
            wait until rising_edge(clk);
            -- write out the output --
            write(Lo, frame, field=>5);  
            write(Lo, string'(" 1 ")); 
            write(Lo, newevent_out); 
            write(Lo, string'(" ")); 
            for i in 0 to NREGIONS-1 loop
                if v_out(i) = '1' then
                    gpart := w72_to_glbparticle(p_out(i));
                else
                    gpart := null_glbparticle;
                end if;
                write(Lo, to_integer(gpart.pt),   field => 5); 
                write(Lo, to_integer(gpart.eta),  field => 5); 
                write(Lo, to_integer(gpart.phi),  field => 5); 
            end loop;
            writeline(Fo, Lo);
            frame := frame + 1;
            --if frame >= 50 then finish(0); end if;
        end loop;
        wait for 50 ns;
        finish(0);
    end process;

    
end Behavioral;
