library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.regionizer_data.all;

entity delay_sort_mux_stream is
    generic(
        NSORTED : natural;
        DELAY  : natural;
        NCLOCKS : natural := NCLK360;
        NREGIONS : natural := NPFREGIONS;
        NSTREAM : natural;
        OUTII : natural := PFII240;
        SORT_NSTAGES : natural := 1
    );
    port(
        ap_clk : IN STD_LOGIC;
        d_in     : IN w72s(NREGIONS-1 downto 0);
        valid_in : IN STD_LOGIC_VECTOR(NREGIONS-1 downto 0);
        roll     : IN STD_LOGIC;
        d_out    : OUT w72s(NSTREAM-1 downto 0);
        roll_out : OUT STD_LOGIC;
        roll_out_tm2 : OUT STD_LOGIC -- anticipates roll_out by 2 clock cycles
    );
end delay_sort_mux_stream;

architecture Behavioral of delay_sort_mux_stream is
    constant EFFDELAY : natural := DELAY-(SORT_NSTAGES-1);

    signal delayed:        w72s(NREGIONS-1 downto 0);
    signal delayed_valid:  std_logic_vector(NREGIONS-1 downto 0) := (others => '0');
    signal delayed_roll:   std_logic := '0';

    signal sorted:        anyparticles(NSORTED*NREGIONS-1 downto 0);
    signal sorted_valid:  std_logic_vector(NSORTED*NREGIONS-1 downto 0) := (others => '0');
    signal sorted_roll:   std_logic_vector(NREGIONS-1 downto 0) := (others => '0');

    signal mux :        anyparticles(NSTREAM-1 downto 0);
    signal mux_valid :  std_logic_vector(NSTREAM-1 downto 0) := (others => '0');
    signal mux_roll :   std_logic := '0';

begin

    maybe_delay: if EFFDELAY > 0 generate
        gen_reg_delay: for i in 0 to NREGIONS-1 generate
            reg_delay: entity work.word_delay
                generic map(DELAY => EFFDELAY, N_BITS => 72)
                port map(clk    => ap_clk, 
                         enable => '1',
                         d      => d_in(i),
                         q      => delayed(i));
            end generate gen_reg_delay;
        valid_delay: entity work.word_delay
                generic map(DELAY => EFFDELAY, N_BITS => NREGIONS)
                port map(clk    => ap_clk, 
                         enable => '1',
                         d      => valid_in,
                         q      => delayed_valid);
        roll_delay: entity work.bit_delay
                generic map(DELAY => EFFDELAY)
                port map(clk    => ap_clk, 
                         enable => '1',
                         d      => roll,
                         q      => delayed_roll);
    else generate
        delayed <= d_in;
        delayed_valid <= valid_in;
        delayed_roll <= roll;
    end generate maybe_delay;


    gen_sorters: for isort in NREGIONS-1 downto 0 generate
        gen_sort: if SORT_NSTAGES = 1 generate
            sorter : entity work.stream_sort
                        generic map(NITEMS => NSORTED, NCLOCKS => NCLOCKS)
                        port map(ap_clk => ap_clk,
                            d_in => w72_to_anyparticle(delayed(isort)),
                            valid_in => delayed_valid(isort),
                            roll => delayed_roll,
                            d_out => sorted((isort+1)*NSORTED-1 downto isort*NSORTED),
                            valid_out => sorted_valid((isort+1)*NSORTED-1 downto isort*NSORTED),
                            roll_out => sorted_roll(isort)
                        );
        else generate
            sorter : entity work.cascade_stream_sort
                        generic map(NITEMS => NSORTED, NSTAGES => SORT_NSTAGES, NCLOCKS => NCLOCKS)
                        port map(ap_clk => ap_clk,
                            d_in => w72_to_anyparticle(delayed(isort)),
                            valid_in => delayed_valid(isort),
                            roll => delayed_roll,
                            d_out => sorted((isort+1)*NSORTED-1 downto isort*NSORTED),
                            valid_out => sorted_valid((isort+1)*NSORTED-1 downto isort*NSORTED),
                            roll_out => sorted_roll(isort)
                        );
            end generate gen_sort;
        end generate gen_sorters;

    roll_out_tm2 <= sorted_roll(0);

    muxer: entity work.region_mux_stream
                    generic map(NREGIONS => NREGIONS, 
                                NITEMS   => NSORTED,
                                NSTREAM  => NSTREAM,
                                OUTII    => OUTII)
                    port map(ap_clk => ap_clk,
                        roll => sorted_roll(0),
                        d_in => sorted,
                        valid_in => sorted_valid,
                        d_out => mux,
                        valid_out => mux_valid,
                        roll_out => mux_roll);

     format: process(ap_clk)
        begin
            if rising_edge(ap_clk) then
                for i in 0 to NSTREAM-1 loop
                    if mux_valid(i) = '1' then
                        d_out(i) <= anyparticle_to_w72(mux(i));
                    else
                        d_out(i) <= (others => '0');
                    end if;
                end loop;
                roll_out <= mux_roll;
            end if;
        end process format;

end Behavioral;
