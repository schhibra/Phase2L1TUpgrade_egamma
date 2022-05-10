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

    signal p_in:  w72s(NCALOSECTORS*NCALOFIBERS-1 downto 0) := (others => (others => '0'));
    signal p_out: w72s(NREGIONS-1 downto 0) := (others => (others => '0'));
    signal v_out: std_logic_vector(NREGIONS-1 downto 0) := (others => '0');

    file Fi : text open read_mode is "input-calo.txt";
    file Fo : text open write_mode is "output-calo-vhdl_tb.txt";


begin
    clk  <= not clk after 1.25 ns;
    
    uut : entity work.calo_regionizer
        port map(ap_clk => clk, 
                 ap_rst => rst, 
                 ap_start => start,
                 ap_ready => ready,
                 ap_idle =>  idle,
                 ap_done => done,
                 calo_in_0_0_V => p_in( 0),
                 calo_in_0_1_V => p_in( 1),
                 calo_in_0_2_V => p_in( 2),
                 calo_in_0_3_V => p_in( 3), 
                 calo_in_1_0_V => p_in( 4),
                 calo_in_1_1_V => p_in( 5),
                 calo_in_1_2_V => p_in( 6),
                 calo_in_1_3_V => p_in( 7),
                 calo_in_2_0_V => p_in( 8),
                 calo_in_2_1_V => p_in( 9), 
                 calo_in_2_2_V => p_in(10),
                 calo_in_2_3_V => p_in(11),
                 calo_out_0_V => p_out(0),
                 calo_out_1_V => p_out(1),
                 calo_out_2_V => p_out(2),
                 calo_out_3_V => p_out(3),
                 calo_out_4_V => p_out(4),
                 calo_out_5_V => p_out(5),
                 calo_out_6_V => p_out(6),
                 calo_out_7_V => p_out(7),
                 calo_out_8_V => p_out(8),
                 calo_out_valid_0 => v_out(0),
                 calo_out_valid_1 => v_out(1),
                 calo_out_valid_2 => v_out(2),
                 calo_out_valid_3 => v_out(3),
                 calo_out_valid_4 => v_out(4),
                 calo_out_valid_5 => v_out(5),
                 calo_out_valid_6 => v_out(6),
                 calo_out_valid_7 => v_out(7),
                 calo_out_valid_8 => v_out(8),
                 newevent => newevent,
                 newevent_out => newevent_out
             );
   

    runit : process 
        variable remainingEvents : integer := 5;
        variable frame : integer := 0;
        variable Li, Lo : line;
        variable itest, iobj : integer;
        variable part : particle;
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
                for i in 0 to NCALOSECTORS*NCALOFIBERS-1  loop
                    read(Li, iobj); part.pt   := (to_unsigned(iobj, 14));
                    read(Li, iobj); part.eta  := (to_signed(iobj, 10));
                    read(Li, iobj); part.phi  := (to_signed(iobj, 10));
                                    part.rest := (others => '0');
                    p_in(i) <= particle_to_w72(part);
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
                    part := w72_to_particle(p_out(i));
                else
                    part := null_particle;
                end if;
                write(Lo, to_integer(part.pt),   field => 5); 
                write(Lo, to_integer(part.eta),  field => 5); 
                write(Lo, to_integer(part.phi),  field => 5); 
            end loop;
            writeline(Fo, Lo);
            frame := frame + 1;
            --if frame >= 50 then finish(0); end if;
        end loop;
        wait for 50 ns;
        finish(0);
    end process;

    
end Behavioral;
